/*
 * Copyright 2015 Follow My Vote, Inc.
 * This file is part of The Follow My Vote Stake-Weighted Voting Application ("SWV").
 *
 * SWV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SWV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SWV.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "CustomEvaluator.hpp"
#include "Contest.hpp"
#include "Decision.hpp"
#include "CoinVolumeHistory.hpp"
#include "Utilities.hpp"

#include <datagram.capnp.h>
#include <decision.capnp.h>
#include <contest.capnp.h>
#include <signed.capnp.h>

#include <capnp/serialize-packed.h>

#include <kj/debug.h>

#include <graphene/chain/database.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/chain/operation_history_object.hpp>

#include <fc/io/raw_variant.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/crypto/digest.hpp>

namespace swv {

void processDecision(gch::database& db, const gch::account_balance_object& balance, ::Decision::Reader decision) {
    KJ_REQUIRE(decision.getOpinions().size() <= 1, "Only single-candidate votes are supported", decision);
    // Recall that contests are ID'd by their operation history ID, not the object ID
    auto& contestIndex = db.get_index_type<ContestIndex>().indices().get<ById>();
    auto itr = contestIndex.find(unpack<gch::operation_history_id_type>(decision.getContest()));
    KJ_REQUIRE(itr != contestIndex.end(), "Decision references unknown contest");
    auto& contest = *itr;
    FC_ASSERT(contest.coin == balance.asset_type, "Publishing balance is in a different coin than the contest",
              ("balance", balance)("contest", contest));
    if (decision.getOpinions().size() == 1) {
        auto opinion = decision.getOpinions()[0];
        KJ_REQUIRE(opinion.getContestant() < contest.contestants.size() + decision.getWriteIns().getEntries().size(),
                   "Opinion references a non-existent contestant", decision, fc::json::to_string(contest));
        KJ_REQUIRE(opinion.getOpinion() == 1, "Only opinions of 1 are supported", decision);
    }

    // TODO: Move tally and untally logic to methods or visitors on Contest
    // Get previous decision
    auto& decisionIndex = db.get_index_type<DecisionIndex>().indices().get<ByVoter>();
    auto decisionItr = decisionIndex.upper_bound(boost::make_tuple(gch::account_balance_id_type(balance.id),
                                                                   contest.contestId));
    if (decisionItr != decisionIndex.end() && decisionItr->opinions.size() > 0) {
        // We use DASSERTs here because we're sanity-checking internal data, which should be guaranteed valid. If it's
        // not, there's a bug somewhere in the code that created it (likely in the call stack of CustomEvaluator, since
        // that's what creates and maintains this data)
        KJ_DASSERT(decisionItr->opinions.size() == 1);
        KJ_DASSERT(contest.coin == decisionItr->voter(db).asset_type);
        // There is an old decision currently in effect. Untally it
        db.modify(contest, [&](Contest& c) {
            auto opinion = *decisionItr->opinions.begin();
            if (opinion.first < c.contestants.size()) {
                // Vote is for a normal candidate
                auto resultItr = c.contestantResults.find(opinion.first);
                KJ_DASSERT(resultItr != c.contestantResults.end());
                resultItr->second -= decisionItr->voter(db).balance.value;
                KJ_DASSERT(resultItr->second >= 0);
            } else {
                // Vote is for a write-in candidate
                KJ_DASSERT(opinion.first - c.contestants.size() < decisionItr->writeIns.size());
                auto writeInName = decisionItr->writeIns[opinion.first - c.contestants.size()].first;
                auto resultItr = c.writeInResults.find(writeInName);
                KJ_DASSERT(resultItr != c.writeInResults.end());
                resultItr->second -= decisionItr->voter(db).balance.value;
                KJ_DASSERT(resultItr->second >= 0);
                // If the write-in no longer has any votes, remove it
                if (resultItr->second == 0)
                    c.writeInResults.erase(resultItr);
            }
        });
    }

    // Store decision and update tally
    auto& newDecision = db.create<Decision>([&db, decision, &contest, balance](Decision& d) {
        auto& index = db.get_index_type<gch::simple_index<gch::operation_history_object>>();
        d.decisionId = gch::operation_history_id_type(index.size() - 1);
        d.voter = balance.id;
        d.contestId = contest.contestId;
        for (auto opinion : decision.getOpinions())
            d.opinions.insert(std::make_pair(opinion.getContestant(), opinion.getOpinion()));
        for (auto writeIn : decision.getWriteIns().getEntries())
            d.writeIns.emplace_back(std::make_pair(writeIn.getKey(), writeIn.getValue()));
    });
    if (newDecision.opinions.size() > 0)
        db.modify(contest, [&](Contest& c) {
            auto opinion = *newDecision.opinions.begin();
            if (opinion.first < c.contestants.size()) {
                // Vote is for a normal candidate
                auto& result = c.contestantResults[opinion.first];
                result += newDecision.voter(db).balance.value;
            } else {
                // Vote is for a write-in candidate
                auto& result = c.writeInResults[newDecision.writeIns[opinion.first - c.contestants.size()].first];
                result += newDecision.voter(db).balance.value;
            }
        });

    // Register decision with coin volume history mechanism
    {
        auto& coinVolumeIndex = db.get_index_type<CoinVolumeHistoryIndex>().indices().get<ByCoin>();
        auto itr = coinVolumeIndex.find(contest.coin);
        if (itr == coinVolumeIndex.end())
            db.create<CoinVolumeHistory>([&newDecision, &contest, &db](CoinVolumeHistory& volumeHistory) {
                volumeHistory.coinId = contest.coin;
                volumeHistory.recordDecision(newDecision, db);
            });
        else
            db.modify(*itr, [&newDecision, &db](CoinVolumeHistory& volumeHistory) {
                volumeHistory.recordDecision(newDecision, db);
            });
    }
}

void processContest(gch::database& db, ::Datagram::ContestKey::Key::Reader key,
                    fc::sha256 contestDigest, ::Contest::Reader contest) {
    // All relevant data consistency checks should have been done before FMV published the contest to the chain. We
    // should be able to skip them here, relying on the FMV signature to be sure this is a legitimate contest creation
    // request.
    db.create<Contest>([&db, key, contestDigest, contest](Contest& c) {
        auto& index = db.get_index_type<gch::simple_index<gch::operation_history_object>>();
        c.contestId = gch::operation_history_id_type(index.size() - 1);
        if (key.isSignature()) {
            auto signaturePack = key.getSignature();
            auto id = unpack<gch::account_id_type>(signaturePack.getId());
            auto signature = unpack<fc::ecc::compact_signature>(signaturePack.getSignature());
            auto creatorKey = fc::ecc::public_key(signature, contestDigest);
            KJ_REQUIRE(id(db).options.memo_key == creatorKey,
                       "Failed to create contest: creator's signature was invalid.");
            c.creator = id;
        } else
            c.creator = GRAPHENE_NULL_ACCOUNT;
        c.name = contest.getName();
        c.description = contest.getDescription();
        c.tags = convertMap(contest.getTags());
        c.contestants = convertMap(contest.getContestants());
        c.coin = gch::asset_id_type(contest.getCoin());
        c.creationTime = db.head_block_time();
        c.startTime = fc::time_point(fc::milliseconds(contest.getStartTime()));
        c.endTime = fc::time_point(fc::milliseconds(contest.getEndTime()));
    });
}

graphene::chain::void_result CustomEvaluator::do_apply(const CustomEvaluator::operation_type& op) {
    kj::ArrayPtr<const kj::byte> data(reinterpret_cast<const kj::byte*>(op.data.data()), op.data.size());
    if (data.size() <= VOTE_MAGIC->size() || data.slice(0, VOTE_MAGIC->size() - 1) != *VOTE_MAGIC)
        // Not my circus, not my monkey.
        return {};

    try {
        BlobMessageReader message(data.slice(VOTE_MAGIC->size(), data.size()));
        auto datagram = message->getRoot<Datagram>();
        BlobMessageReader datagramMessage(datagram.getContent());
        BlobMessageReader keyReader(datagram.getIndex().getKey());

        switch (datagram.getIndex().getType()) {
        case Datagram::DatagramType::DECISION: {
            auto key = keyReader->getRoot<::Datagram::DecisionKey>();
            auto publisherId = gch::account_id_type(key.getBalanceId().getAccountInstance());
            auto assetId = gch::asset_id_type(key.getBalanceId().getCoinInstance());
            KJ_REQUIRE(publisherId == op.fee_payer(),
                       "Nice try. You may not publish decisions for someone else");
            auto& balanceIndex = db().get_index_type<gch::account_balance_index>()
                                     .indices().get<gch::by_account_asset>();
            auto itr = balanceIndex.find(boost::make_tuple(publisherId, assetId));
            KJ_REQUIRE(itr != balanceIndex.end(), "Ignoring decision on a nonexistent balance.");
            processDecision(db(), *itr, datagramMessage->getRoot<::Decision>());
            break;
        }
        case Datagram::DatagramType::CONTEST: {
            KJ_REQUIRE(kj::StringPtr(op.fee_payer()(db()).name) == CONTEST_PUBLISHING_ACCOUNT,
                       "Unauthorized account attempted to publish contest",
                       op.fee_payer()(db()).name, *CONTEST_PUBLISHING_ACCOUNT);
            processContest(db(), keyReader->getRoot<::Datagram::ContestKey>().getKey(), digest(datagram.getContent()),
                           datagramMessage->getRoot<::Contest>());
            break;
        }
        }
    } catch (kj::Exception& e) {
        KJ_LOG(WARNING, "Exception while processing datagram", e, data);
    } catch (fc::exception& e) {
        wlog("Exception while processing datagram", ("exception", e));
        KJ_LOG(WARNING, data);
    }

    return {};
}

} // namespace swv

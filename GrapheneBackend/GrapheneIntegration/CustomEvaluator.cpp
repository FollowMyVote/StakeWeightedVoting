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
#include "Utilities.hpp"
#include "Objects/Contest.hpp"
#include "Objects/Decision.hpp"
#include "Objects/CoinVolumeHistory.hpp"

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
    auto itr = contestIndex.find(gch::operation_history_id_type(decision.getContest().getOperationId()));
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

    // Store decision and update tally
    // NOTE: See issue #143: this tally algorithm is completely wrong, but it gives us fake/test results to play with
    auto& newDecision = db.create<Decision>([&db, decision, &contest, balance](Decision& d) {
        auto& index = db.get_index_type<gch::simple_index<gch::operation_history_object>>();
        d.decisionId = gch::operation_history_id_type(index.size());
        d.voter = balance.id;
        d.contestId = contest.contestId;
        for (auto opinion : decision.getOpinions())
            d.opinions.insert(std::make_pair(opinion.getContestant(), opinion.getOpinion()));
        for (auto writeIn : decision.getWriteIns().getEntries())
            d.writeIns.emplace_back(std::make_pair(writeIn.getKey(), writeIn.getValue()));
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

template <typename T>
inline T unpack(capnp::Data::Reader r) {
    return fc::raw::unpack<T>(std::vector<char>(r.begin(), r.end()));
}

void processContest(gch::database& db, ::Datagram::ContestKey::Creator::Reader key,
                    fc::sha256 contestDigest, ::Contest::Reader contest) {
    // All relevant data consistency checks should have been done before FMV published the contest to the chain. We
    // should be able to skip them here, relying on the FMV signature to be sure this is a legitimate contest creation
    // request.
    const auto& contestObject = db.create<Contest>([&db, key, contestDigest, contest](Contest& c) {
        auto& index = db.get_index_type<gch::simple_index<gch::operation_history_object>>();
        c.contestId = gch::operation_history_id_type(index.size());
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
        c.startTime = std::max(fc::time_point(fc::milliseconds(contest.getStartTime())),
                               fc::time_point(db.head_block_time()));
        c.endTime = fc::time_point(fc::milliseconds(contest.getEndTime()));
    });
    KJ_LOG(DBG, "Created new contest", contestObject.contestId.instance.value);
}

inline fc::sha256 digest(capnp::Data::Reader r) {
    return fc::digest(std::vector<char>(r.begin(), r.end()));
}

graphene::chain::void_result CustomEvaluator::do_apply(const CustomEvaluator::operation_type& op) {
    try {
        kj::ArrayPtr<const kj::byte> data(reinterpret_cast<const kj::byte*>(op.data.data()), op.data.size());
        if (data.size() <= VOTE_MAGIC->size() || data.slice(0, VOTE_MAGIC->size()) != *VOTE_MAGIC)
            // Not my circus, not my monkey.
            return {};
        // Cut off the magic
        data = data.slice(VOTE_MAGIC->size(), data.size());

        try {
            KJ_LOG(DBG, "Found vote custom operation", fc::json::to_pretty_string(op));
            BlobMessageReader message(data);
            auto datagram = message->getRoot<Datagram>();
            BlobMessageReader datagramMessage(datagram.getContent());

            switch (datagram.getKey().getKey().which()) {
            case Datagram::DatagramKey::Key::DECISION_KEY: {
                auto content = datagramMessage->getRoot<::Decision>();
                KJ_LOG(DBG, "Custom op is a vote", datagram, content);
                auto key = datagram.getKey().getKey().getDecisionKey();
                auto publisherId = gch::account_id_type(key.getBalanceId().getAccountInstance());
                auto assetId = gch::asset_id_type(key.getBalanceId().getCoinInstance());
                KJ_REQUIRE(publisherId == op.fee_payer(),
                           "Nice try. You may not publish decisions for someone else");
                auto& balanceIndex = db().get_index_type<gch::account_balance_index>()
                                     .indices().get<gch::by_account_asset>();
                auto itr = balanceIndex.find(boost::make_tuple(publisherId, assetId));
                KJ_REQUIRE(itr != balanceIndex.end(), "Ignoring decision on a nonexistent balance.");
                processDecision(db(), *itr, content);
                break;
            }
            case Datagram::DatagramKey::Key::CONTEST_KEY: {
                auto content = datagramMessage->getRoot<::Contest>();
                KJ_LOG(DBG, "Custom op is a contest", datagram, content);
                KJ_REQUIRE(kj::StringPtr(op.fee_payer()(db()).name) == CONTEST_PUBLISHING_ACCOUNT,
                           "Unauthorized account attempted to publish contest",
                           op.fee_payer()(db()).name, *CONTEST_PUBLISHING_ACCOUNT);
                processContest(db(), datagram.getKey().getKey().getContestKey().getCreator(),
                               digest(datagram.getContent()), content);
                break;
            }
            }
        } catch (kj::Exception& e) {
            KJ_LOG(ERROR, "Exception while processing datagram", e, data);
        } catch (fc::exception& e) {
            edump((e));
            KJ_LOG(ERROR, data);
        }

        return {};
    } FC_CAPTURE_AND_RETHROW((op))
}

} // namespace swv

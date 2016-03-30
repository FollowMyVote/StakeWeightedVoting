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

#include <datagram.capnp.h>
#include <decision.capnp.h>

#include <capnp/serialize-packed.h>

#include <kj/debug.h>

#include <graphene/chain/database.hpp>

namespace swv {

template <typename T>
inline T unpack(capnp::Data::Reader r) {
    return fc::raw::unpack<T>(std::vector<char>(r.begin(), r.end()));
}

void processDecision(gch::database& db, gch::account_balance_id_type publisherId, Decision::Reader decision) {
    KJ_REQUIRE(decision.getOpinions().size() <= 1, "Only single-candidate votes are supported.", decision);
    // Recall that contests are ID'd by their operation history ID, not the object ID
    auto& contestIndex = db.get_index_type<ContestIndex>().indices().get<ById>();
    auto itr = contestIndex.find(unpack<gch::operation_history_id_type>(decision.getContest()));
    KJ_REQUIRE(itr != contestIndex.end(), "Decision references unknown contest");
    auto& contest = *itr;
    // TODO: Untally any votes which are being replaced, and tally the new votes
}

graphene::chain::void_result CustomEvaluator::do_apply(const CustomEvaluator::operation_type& op) {
    kj::ArrayPtr<const kj::byte> data(reinterpret_cast<const kj::byte*>(op.data.data()), op.data.size());
    if (data.size() <= VOTE_MAGIC->size() || data.slice(0, VOTE_MAGIC->size() - 1) != *VOTE_MAGIC)
        // Not my circus, not my monkey.
        return {};

    try {
        kj::ArrayInputStream dataStream(data.slice(VOTE_MAGIC->size(), data.size()));
        capnp::PackedMessageReader message(dataStream);
        auto datagram = message.getRoot<Datagram>();

        switch (datagram.getIndex().getType()) {
        case Datagram::DatagramType::DECISION: {
            kj::ArrayInputStream datagramContents(datagram.getContent());
            capnp::PackedMessageReader datagramMessage(datagramContents);
            auto publisherId = unpack<gch::account_balance_id_type>(datagram.getIndex().getKey());
            KJ_REQUIRE(publisherId(db()).owner == op.fee_payer(),
                       "Nice try. You may not publish decisions for someone else");
            processDecision(db(), publisherId, datagramMessage.getRoot<Decision>());
            break;
        }
        case Datagram::DatagramType::CONTEST:
            // TODO
            break;
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

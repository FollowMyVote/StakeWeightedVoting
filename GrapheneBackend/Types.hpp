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
#ifndef TYPES_HPP
#define TYPES_HPP

#include <graphene/chain/protocol/types.hpp>
#include <graphene/db/generic_index.hpp>

#ifndef VOTE_SPACE_ID
#define VOTE_SPACE_ID 7
#endif

namespace graphene { namespace chain { class database; } }

namespace swv {
namespace gch = graphene::chain;
namespace gdb = graphene::db;

namespace ObjectIds {
enum VoteObjectTypes {
    Contest = 1,
    Decision,
    CoinVolumeHistory
};
}

class Contest;
class Decision;
class CoinVolumeHistory;

using ContestObjectId = gdb::object_id<VOTE_SPACE_ID, ObjectIds::Contest, Contest>;
using DecisionObjectId = gdb::object_id<VOTE_SPACE_ID, ObjectIds::Decision, Decision>;
using CoinVolumeHistoryObjectId = gdb::object_id<VOTE_SPACE_ID, ObjectIds::CoinVolumeHistory, CoinVolumeHistory>;

namespace bmi = boost::multi_index;
struct ById;
}

#endif // TYPES_HPP

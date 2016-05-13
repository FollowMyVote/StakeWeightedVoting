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
#ifndef VOTEDATABASE_HPP
#define VOTEDATABASE_HPP

#include "Contest.hpp"
#include "Decision.hpp"
#include "CoinVolumeHistory.hpp"

#include <graphene/chain/database.hpp>

#include <kj/debug.h>

#define INDEX_GETTER(name) \
    auto& name() { \
        KJ_ASSERT(_ ## name != nullptr, "Not yet initialized: call registerIndexes first"); \
        return *_ ## name; \
    } \
    auto& name() const { \
        KJ_ASSERT(_ ## name != nullptr, "Not yet initialized: call registerIndexes first"); \
        return *_ ## name; \
    }


namespace swv {

/**
 * @brief The VoteDatabase class monitors the blockchain and maintains a database of all voting-related content
 */
class VoteDatabase
{
    gch::database& chain;
    gdb::primary_index<ContestIndex>* _contestIndex = nullptr;
    gdb::primary_index<DecisionIndex>* _decisionIndex = nullptr;
    gdb::primary_index<CoinVolumeHistoryIndex>* _coinVolumeHistoryIndex = nullptr;
public:
    VoteDatabase(gch::database& chain);

    void registerIndexes();

    gch::database& db() {
        return chain;
    }
    const gch::database& db() const {
        return chain;
    }

    INDEX_GETTER(contestIndex)
    INDEX_GETTER(decisionIndex)
    INDEX_GETTER(coinVolumeHistoryIndex)
};

} // namespace swv

#undef INDEX_GETTER
#endif // VOTEDATABASE_HPP

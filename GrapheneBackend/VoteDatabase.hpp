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

#include "Objects/Contest.hpp"
#include "Objects/Decision.hpp"
#include "Objects/CoinVolumeHistory.hpp"
#include "BackendConfiguration.hpp"

#include <graphene/chain/database.hpp>
#include <graphene/net/node.hpp>

#include <kj/debug.h>

#include <boost/signals2.hpp>

#define GETTERS(name) \
    auto& name() { \
        KJ_ASSERT(_ ## name != nullptr, "Not yet initialized: call registerIndexes first"); \
        return *_ ## name; \
    } \
    auto& name() const { \
        KJ_ASSERT(_ ## name != nullptr, "Not yet initialized: call registerIndexes first"); \
        return *_ ## name; \
    }


namespace swv {
class CustomEvaluator;

/**
 * @brief The VoteDatabase class monitors the blockchain and maintains a database of all voting-related content
 */
class VoteDatabase
{
    gch::database& chain;
    graphene::net::node_ptr p2p_node;
    CustomEvaluator* _customEvaluator = nullptr;
    gdb::primary_index<ContestIndex>* _contestIndex = nullptr;
    gdb::primary_index<DecisionIndex>* _decisionIndex = nullptr;
    gdb::primary_index<CoinVolumeHistoryIndex>* _coinVolumeHistoryIndex = nullptr;
    BackendConfiguration config;

    class ResultUpdateWatcher : public gdb::secondary_index {
        VoteDatabase* vdb = nullptr;
    public:
        ResultUpdateWatcher() {}

        void setVoteDatabase(VoteDatabase* vdb) {
            this->vdb = vdb;
        }

        // secondary_index interface
        virtual void object_modified(const graphene::db::object& after) override;
    };

public:
    VoteDatabase(gch::database& chain);

    void registerIndexes();
    void startup(graphene::net::node_ptr node);

    gch::database& db() {
        return chain;
    }
    const gch::database& db() const {
        return chain;
    }
    graphene::net::node& node() {
        return *p2p_node;
    }
    const graphene::net::node& node() const {
        return *p2p_node;
    }

    GETTERS(customEvaluator)
    GETTERS(contestIndex)
    GETTERS(decisionIndex)
    GETTERS(coinVolumeHistoryIndex)

    BackendConfiguration& configuration() {
        return config;
    }
    const BackendConfiguration& configuration() const {
        return config;
    }

    boost::signals2::signal<void(gch::operation_history_id_type)> contestResultsUpdated;
};

} // namespace swv

#undef GETTERS
#endif // VOTEDATABASE_HPP

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
#include "VoteDatabase.hpp"
#include "CustomEvaluator.hpp"
#include "Contest.hpp"
#include "Decision.hpp"

namespace swv {

VoteDatabase::VoteDatabase(gch::database& chain, graphene::net::node& p2p_node)
    : chain(chain), p2p_node(p2p_node) {}

void VoteDatabase::registerIndexes() {
    chain.register_evaluator<CustomEvaluator>();
    _contestIndex = chain.add_index<gdb::primary_index<ContestIndex>>();
    // If build fails on this next line, it's because https://github.com/cryptonomex/graphene/pull/653 hasn't been
    // merged yet. You will need that patch in order to build this.
    _contestIndex->add_secondary_index<ResultUpdateWatcher>()->setVoteDatabase(this);
    _decisionIndex = chain.add_index<gdb::primary_index<DecisionIndex>>();
    _coinVolumeHistoryIndex = chain.add_index<gdb::primary_index<CoinVolumeHistoryIndex>>();
}

void VoteDatabase::startup() {
    config.open((chain.get_data_dir() / "configuration.bin").preferred_string().c_str());
}

void VoteDatabase::ResultUpdateWatcher::object_modified(const graphene::db::object& after) {
    if (vdb == nullptr)
        return;
    auto contest = dynamic_cast<const Contest*>(&after);
    if (contest != nullptr)
        vdb->contestResultsUpdated(contest->contestId);
}

} // namespace swv

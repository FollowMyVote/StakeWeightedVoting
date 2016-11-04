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
#include "GrapheneIntegration/CustomEvaluator.hpp"
#include "Objects/Contest.hpp"
#include "Objects/Decision.hpp"

#include <fc/smart_ref_impl.hpp>

namespace swv {

VoteDatabase::VoteDatabase(gch::database& chain)
    : chain(chain) {
}

void VoteDatabase::initialize(const fc::path& dataDir) {
    if (!fc::exists(dataDir / "configuration.bin") && fc::exists(dataDir / "blockchain" / "configuration.bin")) {
        KJ_DBG("Moving Follow My Vote configuration to new location.");
        fc::rename((dataDir / "blockchain" / "configuration.bin"), (dataDir / "configuration.bin"));
    }
    config.open((dataDir / "configuration.bin").preferred_string().c_str());
    CustomEvaluator::setVoteDatabase(*this);
    chain.register_evaluator<CustomEvaluator>();
}

void VoteDatabase::registerIndexes() {
    _contestIndex = chain.add_index<gdb::primary_index<ContestIndex>>();
    // If build fails on this next line, it's because https://github.com/cryptonomex/graphene/pull/653 hasn't been
    // merged yet. You will need that patch in order to build this.
    _contestIndex->add_secondary_index<ResultUpdateWatcher>()->setVoteDatabase(this);
    _decisionIndex = chain.add_index<gdb::primary_index<DecisionIndex>>();
    _coinVolumeHistoryIndex = chain.add_index<gdb::primary_index<CoinVolumeHistoryIndex>>();
}

void VoteDatabase::startup(graphene::net::node_ptr node) {
    p2p_node = node;
}

void VoteDatabase::ResultUpdateWatcher::object_modified(const graphene::db::object& after) {
    if (vdb == nullptr)
        return;
    auto contest = dynamic_cast<const Contest*>(&after);
    if (contest != nullptr)
        vdb->contestResultsUpdated(contest->contestId);
}

} // namespace swv

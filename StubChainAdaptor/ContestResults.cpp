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
#include "ContestResults.hpp"

namespace swv {

swv::ContestResults::ContestResults(std::map<int32_t, int64_t> contestantTallies, std::map<kj::String, int64_t> writeInTallies)
    : contestantTallies(kj::mv(contestantTallies)),
      writeInTallies(kj::mv(writeInTallies))
{}

ContestResults::~ContestResults()
{}

::kj::Promise<void> swv::ContestResults::results(Backend::ContestResults::Server::ResultsContext context)
{
    auto results = context.initResults().initResults(contestantTallies.size() + writeInTallies.size());
    int index = 0;

    for (const auto& tally : contestantTallies) {
        auto result = results[index++];
        result.initContestant().setContestant(tally.first);
        result.setTally(tally.second);
    }
    for (const auto& tally : writeInTallies) {
        auto result = results[index++];
        result.initContestant().setWriteIn(tally.first);
        result.setTally(tally.second);
    }

    return kj::READY_NOW;
}

::kj::Promise<void> swv::ContestResults::subscribe(Backend::ContestResults::Server::SubscribeContext context)
{
    // TODO
    return kj::READY_NOW;
}

} // namespace swv

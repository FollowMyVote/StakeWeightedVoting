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
#include "ContestGenerator.hpp"

#include <numeric>
#include <limits>

#include <kj/debug.h>

swv::ContestGenerator::ContestGenerator(int contestCount)
    : contestCount(contestCount) {
    KJ_REQUIRE(contestCount <= std::numeric_limits<char>::max());
}

swv::ContestGenerator::~ContestGenerator()
{}

::kj::Promise<void> swv::ContestGenerator::getContest(ContestGenerator::Server::GetContestContext context)
{
    KJ_REQUIRE(generated < contestCount, "No more contests available.");
    auto contest = context.initResults().initNextContest();
    contest.initContestId().setOperationId(contestCount - generated++);
    contest.setTracksLiveResults(false);
    contest.setVotingStake(0);
    return kj::READY_NOW;
}

::kj::Promise<void> swv::ContestGenerator::getContests(ContestGenerator::Server::GetContestsContext context)
{
    auto count = std::min<int>(contestCount - generated, context.getParams().getCount());
    auto resultContests = context.initResults().initNextContests(count);
    for (auto contest : resultContests) {
        contest.initContestId().setOperationId(contestCount - generated++);
        contest.setTracksLiveResults(false);
        contest.setVotingStake(0);
    }
    return kj::READY_NOW;
}

::kj::Promise<void> swv::ContestGenerator::logEngagement(ContestGenerator::Server::LogEngagementContext)
{
    // Currently a nop
    return kj::READY_NOW;
}

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

#include "ContestGeneratorImpl.hpp"

#include <kj/debug.h>

ContestGeneratorImpl::ContestGeneratorImpl()
{}

ContestGeneratorImpl::~ContestGeneratorImpl()
{}

::kj::Promise<void> ContestGeneratorImpl::getContest(ContestGenerator::Server::GetContestContext context)
{
    auto contest = context.getResults().initNextContest();
    populateContest(contest);

    return kj::READY_NOW;
}

::kj::Promise<void> ContestGeneratorImpl::getContests(ContestGenerator::Server::GetContestsContext context)
{
    auto contestCount = kj::min(context.getParams().getCount(), kj::max(0, 10 - fetched));
    auto contests = context.getResults().initNextContests(static_cast<unsigned>(contestCount));

    for (auto builder : contests)
        populateContest(builder);

    return kj::READY_NOW;
}

void ContestGeneratorImpl::populateContest(ContestGenerator::ListedContest::Builder contest)
{
    switch(fetched++) {
    case 0:
        contest.initContestId(1)[0] = 0;
        contest.setVotingStake(10);
        contest.setTracksLiveResults(true);
        break;
    default:
        KJ_REQUIRE(fetched <= 10, "No more contests are available.");
        contest.initContestId(1)[0] = static_cast<unsigned char>(fetched - 1);
        contest.setVotingStake(80000000000);
        contest.setTracksLiveResults(false);
        break;
    }
}

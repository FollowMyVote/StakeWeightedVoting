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

#include "BackendServer.hpp"
#include "ContestCreatorImpl.hpp"
#include "ContestGeneratorImpl.hpp"
#include "PurchaseImpl.hpp"

#include <kj/debug.h>

#include <unistd.h>
#include <iostream>

BackendServer::BackendServer()
{}

::kj::Promise<void> BackendServer::getContestFeed(GetContestFeedContext context)
{
    context.getResults().setGenerator(kj::heap<ContestGeneratorImpl>());
    return kj::READY_NOW;
}

::kj::Promise<void> BackendServer::searchContests(Backend::Server::SearchContestsContext context)
{
    context.getResults().setGenerator(kj::heap<ContestGeneratorImpl>());
    return kj::READY_NOW;
}

::kj::Promise<void> BackendServer::getContestResults(Backend::Server::GetContestResultsContext context)
{
    auto contestId = context.getParams().getContestId()[0];
    auto results = context.getResults();
    switch(contestId) {
    case 0:
        results.setResults(kj::heap<ContestResultsImpl>(QMap<qint32, qint64>({{0,10},{1,88}})));
        break;
    case 1:
        results.setResults(kj::heap<ContestResultsImpl>(QMap<qint32, qint64>({{0,10000},{1,2}})));
        break;
    default:
        KJ_FAIL_REQUIRE("Unknown contest ID.", contestId);
    }

    return kj::READY_NOW;
}

::kj::Promise<void> BackendServer::getContestCreator(Backend::Server::GetContestCreatorContext context)
{
    context.getResults().setCreator(kj::heap<ContestCreatorImpl>());
    return kj::READY_NOW;
}

::kj::Promise<void> ContestResultsImpl::results(Backend::ContestResults::Server::ResultsContext context)
{
    auto results = context.getResults().initResults(contestResults.size());
    auto mapResults = contestResults;
    for (unsigned i = 0; i < results.size(); ++i) {
        results[i].setContestant(mapResults.firstKey());
        results[i].setTally(mapResults.take(mapResults.firstKey()));
    }
    return kj::READY_NOW;
}

::kj::Promise<void> ContestResultsImpl::subscribe(Backend::ContestResults::Server::SubscribeContext)
{
    // For now, results never update in the stub server.
    return kj::READY_NOW;
}

::kj::Promise<void> PurchaseImpl::complete(Purchase::Server::CompleteContext context)
{
    context.getResults().setResult(isComplete);
    return kj::READY_NOW;
}

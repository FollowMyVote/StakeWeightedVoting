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
#include "BackendStub.hpp"
#include "ContestGenerator.hpp"

namespace swv {

StubChainAdaptor::BackendStub::BackendStub(StubChainAdaptor &adaptor)
    : adaptor(adaptor)
{}

StubChainAdaptor::BackendStub::~BackendStub()
{}

::kj::Promise<void> StubChainAdaptor::BackendStub::getContestFeed(Backend::Server::GetContestFeedContext context)
{
    std::vector<Contest::Reader> feedContests;
    feedContests.reserve(adaptor.contests.size());

    for (const auto& contest : adaptor.contests)
        feedContests.emplace_back(contest.getReader());

    context.initResults().setGenerator(kj::heap<swv::ContestGenerator>(kj::mv(feedContests)));
    return kj::READY_NOW;
}

::kj::Promise<void> StubChainAdaptor::BackendStub::searchContests(Backend::Server::SearchContestsContext context)
{
    std::vector<Contest::Reader> feedContests;
    feedContests.reserve(adaptor.contests.size());

    //TODO: implement filtering
    for (const auto& contest : adaptor.contests)
        feedContests.emplace_back(contest.getReader());

    context.initResults().setGenerator(kj::heap<swv::ContestGenerator>(kj::mv(feedContests)));
    return kj::READY_NOW;
}

::kj::Promise<void> StubChainAdaptor::BackendStub::getContestResults(Backend::Server::GetContestResultsContext context)
{

}

::kj::Promise<void> StubChainAdaptor::BackendStub::getContestCreator(Backend::Server::GetContestCreatorContext context)
{

}

::kj::Promise<void> StubChainAdaptor::BackendStub::getCoinDetails(Backend::Server::GetCoinDetailsContext context)
{

}

}

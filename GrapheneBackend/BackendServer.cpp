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

BackendServer::BackendServer() {}
BackendServer::~BackendServer() {}


::kj::Promise<void> BackendServer::getContestFeed(Backend::Server::GetContestFeedContext context) {
}

::kj::Promise<void> BackendServer::searchContests(Backend::Server::SearchContestsContext context) {
}

::kj::Promise<void> BackendServer::getContestResults(Backend::Server::GetContestResultsContext context) {
}

::kj::Promise<void> BackendServer::createContest(Backend::Server::CreateContestContext context) {
}

::kj::Promise<void> BackendServer::getCoinDetails(Backend::Server::GetCoinDetailsContext context) {
}
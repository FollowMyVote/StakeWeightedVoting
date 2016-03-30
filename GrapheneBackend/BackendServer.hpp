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
#ifndef BACKENDSERVER_HPP
#define BACKENDSERVER_HPP

#include <capnp/backend.capnp.h>

class BackendServer : public Backend::Server
{
public:
    BackendServer();
    virtual ~BackendServer();

    // Backend::Server interface
protected:
    virtual ::kj::Promise<void> getContestFeed(GetContestFeedContext context) override;
    virtual ::kj::Promise<void> searchContests(SearchContestsContext context) override;
    virtual ::kj::Promise<void> getContestResults(GetContestResultsContext context) override;
    virtual ::kj::Promise<void> createContest(CreateContestContext context) override;
    virtual ::kj::Promise<void> getCoinDetails(GetCoinDetailsContext context) override;
};

#endif // BACKENDSERVER_HPP

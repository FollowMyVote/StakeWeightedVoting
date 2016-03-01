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
#ifndef BACKENDSTUB_HPP
#define BACKENDSTUB_HPP

#include "StubChainAdaptor.hpp"

#include <capnp/backend.capnp.h>

namespace swv {

/*!
 * \brief The BackendStub class works in conjunction with the StubChainAdaptor to emulate a fully functioning blockchain
 */
class StubChainAdaptor::BackendStub : public ::Backend::Server
{
public:
    virtual ~BackendStub();

    // Backend::Server interface
protected:
    ::kj::Promise<void> getContestFeed(GetContestFeedContext context);
    ::kj::Promise<void> searchContests(SearchContestsContext context);
    ::kj::Promise<void> getContestResults(GetContestResultsContext context);
    ::kj::Promise<void> getContestCreator(GetContestCreatorContext context);
    ::kj::Promise<void> getCoinDetails(GetCoinDetailsContext context);

private:
    // Only StubChainAdaptor::getBackendStub() can instantiate a BackendStub.
    friend BackendStub StubChainAdaptor::getBackendStub();
    BackendStub(StubChainAdaptor& adaptor);

    StubChainAdaptor& adaptor;
};

} // namespace swv

#endif // BACKENDSTUB_HPP

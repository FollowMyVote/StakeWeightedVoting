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
#include "ContestCreatorServer.hpp"

namespace swv {

ContestCreatorServer::ContestCreatorServer(VoteDatabase& vdb)
    : vdb(vdb) {}

ContestCreatorServer::~ContestCreatorServer() {}

::kj::Promise<void> ContestCreatorServer::getPriceSchedule(ContestCreator::Server::GetPriceScheduleContext context) {

}

::kj::Promise<void> ContestCreatorServer::getContestLimits(ContestCreator::Server::GetContestLimitsContext context) {

}

::kj::Promise<void> ContestCreatorServer::purchaseContest(ContestCreator::Server::PurchaseContestContext context) {

}

} // namespace swv

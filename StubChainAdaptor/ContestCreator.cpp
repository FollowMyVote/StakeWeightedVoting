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
#include "ContestCreator.hpp"

namespace swv {

ContestCreator::ContestCreator()
{}

ContestCreator::~ContestCreator()
{}

::kj::Promise<void> ContestCreator::getPriceSchedule(ContestCreator::Server::GetPriceScheduleContext context) {
    auto entries = context.getResults().getSchedule().initEntries(8);
    entries[0].getKey().setItem(::ContestCreator::LineItems::CONTEST_TYPE_ONE_OF_N);
    entries[0].getValue().setPrice(40000);
    entries[1].getKey().setItem(::ContestCreator::LineItems::PLURALITY_TALLY);
    entries[1].getValue().setPrice(10000);
    entries[2].getKey().setItem(::ContestCreator::LineItems::CONTESTANT3);
    entries[2].getValue().setPrice(5000);
    entries[3].getKey().setItem(::ContestCreator::LineItems::CONTESTANT4);
    entries[3].getValue().setPrice(5000);
    entries[4].getKey().setItem(::ContestCreator::LineItems::CONTESTANT5);
    entries[4].getValue().setPrice(2500);
    entries[5].getKey().setItem(::ContestCreator::LineItems::CONTESTANT6);
    entries[5].getValue().setPrice(2500);
    entries[6].getKey().setItem(::ContestCreator::LineItems::CONTESTANT7_PLUS);
    entries[6].getValue().setPrice(2000);
    entries[7].getKey().setItem(::ContestCreator::LineItems::INFINITE_DURATION_CONTEST);
    entries[7].getValue().setPrice(50000);
    return kj::READY_NOW;

}

::kj::Promise<void> ContestCreator::getContestLimits(ContestCreator::Server::GetContestLimitsContext context) {
    auto entries = context.getResults().getLimits().initEntries(8);
    entries[0].getKey().setLimit(::ContestCreator::ContestLimits::NAME_LENGTH);
    entries[0].getValue().setValue(100);
    entries[1].getKey().setLimit(::ContestCreator::ContestLimits::DESCRIPTION_SOFT_LENGTH);
    entries[1].getValue().setValue(500);
    entries[2].getKey().setLimit(::ContestCreator::ContestLimits::DESCRIPTION_HARD_LENGTH);
    entries[2].getValue().setValue(10240);
    entries[3].getKey().setLimit(::ContestCreator::ContestLimits::CONTESTANT_COUNT);
    entries[3].getValue().setValue(8);
    entries[4].getKey().setLimit(::ContestCreator::ContestLimits::CONTESTANT_NAME_LENGTH);
    entries[4].getValue().setValue(30);
    entries[5].getKey().setLimit(::ContestCreator::ContestLimits::CONTESTANT_DESCRIPTION_SOFT_LENGTH);
    entries[5].getValue().setValue(500);
    entries[6].getKey().setLimit(::ContestCreator::ContestLimits::CONTESTANT_DESCRIPTION_HARD_LENGTH);
    entries[6].getValue().setValue(10240);
    entries[7].getKey().setLimit(::ContestCreator::ContestLimits::MAX_END_DATE);
    entries[7].getValue().setValue(0);
    return kj::READY_NOW;
}

::kj::Promise<void> ContestCreator::purchaseContest(ContestCreator::Server::PurchaseContestContext context) {
    // TODO
}

} // namespace swv

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
#include "Purchase.hpp"
#include "StubChainAdaptor.hpp"

#include <kj/debug.h>
#include <capnp/serialize-packed.h>

#include <chrono>
#include <map>

namespace swv {

StubChainAdaptor::ContestCreator::ContestCreator(StubChainAdaptor& adaptor)
    : adaptor(adaptor)
{}

StubChainAdaptor::ContestCreator::~ContestCreator()
{}

::kj::Promise<void> StubChainAdaptor::ContestCreator::getPriceSchedule(ContestCreator::Server::GetPriceScheduleContext context) {
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

::kj::Promise<void> StubChainAdaptor::ContestCreator::getContestLimits(ContestCreator::Server::GetContestLimitsContext context) {
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

::kj::Promise<void> StubChainAdaptor::ContestCreator::purchaseContest(ContestCreator::Server::PurchaseContestContext context) {
    int64_t price = 0;
    auto creationRequest = context.getParams().getRequest();
    bool longText = false;

    // Check limits
    KJ_REQUIRE(creationRequest.getContestName().size() > 0, "Contest must have a name", creationRequest);
    KJ_REQUIRE(creationRequest.getContestName().size() <= 100, "Contest name is too long", creationRequest);
    KJ_REQUIRE(creationRequest.getContestDescription().size() <= 10240,
               "Contest description is too long", creationRequest);
    if (creationRequest.getContestDescription().size() > 500)
        longText = true;
    KJ_REQUIRE(creationRequest.getContestants().getEntries().size() > 0, "Contest must have at least one contestant",
               creationRequest);
    KJ_REQUIRE(creationRequest.getContestants().getEntries().size() <= 8,
               "Contest has too many contestants", creationRequest);
    for (auto contestant : creationRequest.getContestants().getEntries()) {
        KJ_REQUIRE(contestant.getKey().size() > 0, "Contestant must have a name", contestant);
        KJ_REQUIRE(contestant.getKey().size() <= 30, "Contestant name is too long", contestant);
        KJ_REQUIRE(contestant.getValue().size() <= 10240, "Contestant description is too long", contestant);
        if (contestant.getValue().size() > 500)
            longText = true;
    }
    auto minimumEndDate = std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::system_clock::now().time_since_epoch() +
                              std::chrono::minutes(10)
                          ).count();
    KJ_REQUIRE(creationRequest.getContestExpiration() == 0 || creationRequest.getContestExpiration() > minimumEndDate,
               "Contest expiration must be at least 10 minutes in the future.", creationRequest);

    switch(creationRequest.getContestType()) {
    case ::ContestCreator::ContestTypes::ONE_OF_N:
        price += 40000;
        break;
    }

    // Count up the base cost
    switch(creationRequest.getTallyAlgorithm()) {
    case ::ContestCreator::TallyAlgorithms::PLURALITY:
        price += 10000;
        break;
    }

    switch(creationRequest.getContestants().getEntries().size()) {
    // Fall-through is intentional
    default: price += (creationRequest.getContestants().getEntries().size() - 6) * 2000;
    case 6: price += 2500;
    case 5: price += 2500;
    case 4: price += 5000;
    case 3: price += 5000;
    case 2:
    case 1:
        break;
    }

    if (creationRequest.getContestExpiration() == 0)
        price += 50000;

    // Calculate surcharges
    std::map<std::string, int64_t> surcharges;
    if (longText) {
        auto charge = ((creationRequest.totalSize().wordCount * capnp::BYTES_PER_WORD) / 1024) * 10000;
        surcharges["Long descriptions"] = charge;
    }

    std::map<std::string, std::string> contestants;
    for (auto contestant : creationRequest.getContestants().getEntries())
        contestants.insert(std::make_pair<std::string, std::string>(contestant.getKey(), contestant.getValue()));
    context.getResults().setPurchaseApi(kj::heap<Purchase>(
                                            price,
                                            KJ_ASSERT_NONNULL(adaptor.getCoinOrphan("VOTE")).getReader().getId(),
                                            [&adaptor = adaptor,
                                             name = std::string(creationRequest.getContestName()),
                                             descripton = std::string(creationRequest.getContestDescription()),
                                             contestants = kj::mv(contestants),
                                             weightCoin = creationRequest.getWeightCoin(),
                                             endTime = creationRequest.getContestExpiration()] {
        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now().time_since_epoch()).count();
        auto contest = adaptor.createContest().getValue();
        contest.setName(name);
        contest.setDescription(descripton);
        auto finalContestants = contest.initContestants().initEntries(contestants.size());
        int index = 0;
        for (auto& contestant : contestants) {
            auto finalContestant = finalContestants[index++];
            finalContestant.setKey(kj::mv(contestant.first));
            finalContestant.setValue(kj::mv(contestant.second));
        }
        contest.setCoin(weightCoin);
        contest.setStartTime(now);
        contest.setEndTime(endTime);
        KJ_LOG(DBG, "Created contest", contest);
    }, kj::mv(surcharges)));

    return kj::READY_NOW;
}

} // namespace swv

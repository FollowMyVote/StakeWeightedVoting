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
#include "VoteDatabase.hpp"
#include "Utilities.hpp"

#include <graphene/chain/protocol/transaction.hpp>

#include <capnp/message.h>
#include <capnp/serialize-packed.h>

#include <datagram.capnp.h>

#include <fc/smart_ref_impl.hpp>

namespace swv {

class PurchaseServer : public ::Purchase::Server {
    VoteDatabase& vdb;
    int64_t votePrice;
    bool oversized;
    capnp::MallocMessageBuilder message;

public:
    PurchaseServer(VoteDatabase& vdb, int64_t votePrice, bool oversized,
                   ContestCreator::ContestCreationRequest::Reader request);
    virtual ~PurchaseServer(){}

protected:
    // Purchase::Server interface
    virtual ::kj::Promise<void> complete(CompleteContext context) override;
    virtual ::kj::Promise<void> prices(PricesContext context) override;
    virtual ::kj::Promise<void> subscribe(SubscribeContext context) override;
    virtual ::kj::Promise<void> paymentSent(PaymentSentContext context) override;
};

ContestCreatorServer::ContestCreatorServer(VoteDatabase& vdb)
    : vdb(vdb) {}

ContestCreatorServer::~ContestCreatorServer() {}

::kj::Promise<void> ContestCreatorServer::getPriceSchedule(ContestCreator::Server::GetPriceScheduleContext context) {
    auto schedule = context.initResults().initSchedule().initEntries(7);
    auto index = 0u;
    for (auto item : vdb.configuration().reader().getPriceSchedule()) {
        auto entry = schedule[index++];
        entry.getKey().setItem(item.getLineItem());
        entry.getValue().setPrice(item.getPrice());
    }
    return kj::READY_NOW;
}

::kj::Promise<void> ContestCreatorServer::getContestLimits(ContestCreator::Server::GetContestLimitsContext context) {
    auto schedule = context.initResults().initLimits().initEntries(7);
    auto index = 0u;
    for (auto item : vdb.configuration().reader().getContestLimits()) {
        auto entry = schedule[index++];
        entry.getKey().setLimit(item.getName());
        entry.getValue().setValue(item.getLimit());
    }
    return kj::READY_NOW;
}

::kj::Promise<void> ContestCreatorServer::purchaseContest(ContestCreator::Server::PurchaseContestContext context) {
    int64_t price = 0;
    auto contestOptions = context.getParams().getRequest().getContestOptions();
    auto config = vdb.configuration().reader();
    std::map<ContestCreator::ContestLimits, int64_t> limits;
    std::transform(config.getContestLimits().begin(), config.getContestLimits().end(),
                   std::inserter(limits, limits.begin()), [](Config::ContestLimit::Reader limit) {
        return std::make_pair(limit.getName(), limit.getLimit());
    });
    std::map<ContestCreator::LineItems, int64_t> prices;
    std::transform(config.getPriceSchedule().begin(), config.getPriceSchedule().end(),
                   std::inserter(prices, prices.begin()), [](Config::Price::Reader limit) {
        return std::make_pair(limit.getLineItem(), limit.getPrice());
    });
#define LIMIT(limit) limits[ContestCreator::ContestLimits::limit]
#define PRICE(price) prices[ContestCreator::LineItems::price]
    bool longText = false;

    // Check limits
    KJ_REQUIRE(contestOptions.getName().size() > 0, "Contest must have a name", contestOptions);
    KJ_REQUIRE(contestOptions.getName().size() <= LIMIT(NAME_LENGTH),
            "Contest name is too long", contestOptions);
    KJ_REQUIRE(contestOptions.getDescription().size() <= LIMIT(DESCRIPTION_HARD_LENGTH),
               "Contest description is too long", contestOptions);
    if (contestOptions.getDescription().size() > LIMIT(DESCRIPTION_SOFT_LENGTH))
        longText = true;
    KJ_REQUIRE(contestOptions.getContestants().getEntries().size() > 0, "Contest must have at least one contestant",
               contestOptions);
    KJ_REQUIRE(contestOptions.getContestants().getEntries().size() <= LIMIT(CONTESTANT_COUNT),
               "Contest has too many contestants", contestOptions);
    for (auto contestant : contestOptions.getContestants().getEntries()) {
        KJ_REQUIRE(contestant.getKey().size() > 0, "Contestant must have a name", contestant);
        KJ_REQUIRE(contestant.getKey().size() <= LIMIT(CONTESTANT_NAME_LENGTH),
                   "Contestant name is too long", contestant);
        KJ_REQUIRE(contestant.getValue().size() <= LIMIT(CONTESTANT_DESCRIPTION_HARD_LENGTH),
                   "Contestant description is too long", contestant);
        if (contestant.getValue().size() > LIMIT(CONTESTANT_DESCRIPTION_SOFT_LENGTH))
            longText = true;
    }
    auto minimumEndDate = std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::system_clock::now().time_since_epoch() +
                              std::chrono::minutes(10)
                          ).count();
    KJ_REQUIRE(contestOptions.getEndTime() == 0 || contestOptions.getEndTime() > minimumEndDate,
               "Contest end time must be at least 10 minutes in the future.", contestOptions);

    switch(contestOptions.getType()) {
    case ::Contest::Type::ONE_OF_N:
        price += PRICE(CONTEST_TYPE_ONE_OF_N);
        break;
    }

    // Count up the base cost
    switch(contestOptions.getTallyAlgorithm()) {
    case ::Contest::TallyAlgorithm::PLURALITY:
        price += PRICE(PLURALITY_TALLY);
        break;
    }

    switch(contestOptions.getContestants().getEntries().size()) {
    // Fall-through is intentional
    default: price += (contestOptions.getContestants().getEntries().size() - 6) * PRICE(CONTESTANT7_PLUS);
    case 6: price += PRICE(CONTESTANT6);
    case 5: price += PRICE(CONTESTANT5);
    case 4: price += PRICE(CONTESTANT4);
    case 3: price += PRICE(CONTESTANT3);
    case 2:
    case 1:
        break;
    }

    if (contestOptions.getEndTime() == 0)
        price += PRICE(INFINITE_DURATION_CONTEST);

    context.getResults().setPurchaseApi(kj::heap<PurchaseServer>(vdb, price, longText,
                                                                 context.getParams().getRequest()));

    return kj::READY_NOW;
#undef LIMIT
#undef PRICE
}

PurchaseServer::PurchaseServer(VoteDatabase& vdb, int64_t votePrice, bool oversized,
                               ContestCreator::ContestCreationRequest::Reader request)
    : vdb(vdb), votePrice(votePrice), oversized(oversized) {
    auto datagram = message.initRoot<Datagram>();
    datagram.initIndex().setType(Datagram::DatagramType::CONTEST);

    {
        ReaderPacker packer(request.getCreatorSignature());
        datagram.getIndex().setKey(packer.array());
    } {
        ReaderPacker packer(request.getContestOptions());
        datagram.setContent(packer.array());
    }
}

::kj::Promise<void> PurchaseServer::complete(Purchase::Server::CompleteContext context) {

}

::kj::Promise<void> PurchaseServer::prices(Purchase::Server::PricesContext context) {
    gch::custom_operation op;
    auto& accountIndex = vdb.db().get_index_type<gch::account_index>().indices().get<gch::by_name>();
    auto publisher = accountIndex.find(std::string(CONTEST_PUBLISHING_ACCOUNT.get()));
    KJ_ASSERT(publisher != accountIndex.end(), "Wat? Contest publishing account is not registered?...");
    op.payer = publisher->id;
    auto& assetIndex = vdb.db().get_index_type<gch::asset_index>().indices().get<gch::by_symbol>();
    auto vote = assetIndex.find("VOTE");
    KJ_ASSERT(vote != assetIndex.end(), "Wat? VOTE is not registered?...");

    // Why do I have to explicitly cast MallocMessageBuilder to MessageBuilder&? Are you drunk, compiler?
    ReaderPacker packer((capnp::MessageBuilder&)message);
    op.data.resize(packer.array().size());
    memcpy(op.data.data(), packer.array().begin(), op.data.size());

    // Calculate surcharges
    std::map<std::string, int64_t> adjustments;
    if (oversized) {
        auto fee = op.calculate_fee(vdb.db().current_fee_schedule().get<gch::custom_operation>());
        auto charge = gch::asset(fee) * vote->options.core_exchange_rate;
        adjustments["Oversized contest fee"] = charge.amount.value;
        votePrice += charge.amount.value;
    }

    // TODO: handle sponsorships
    // TODO: handle promo codes
    auto price = context.initResults().initPrices(1)[0];
    price.setCoinId(vote->id.instance());
    price.setAmount(votePrice);
    price.setPayAddress(publisher->name);

    auto finalAdjustments = context.getResults().initAdjustments().initEntries(adjustments.size());
    auto index = 0;
    for (const auto& adjustment : adjustments) {
        auto finalSurcharge = finalAdjustments[index++];
        finalSurcharge.setKey(adjustment.first);
        finalSurcharge.initValue().setPrice(adjustment.second);
    }

    return kj::READY_NOW;
}

::kj::Promise<void> PurchaseServer::subscribe(Purchase::Server::SubscribeContext context) {

}

::kj::Promise<void> PurchaseServer::paymentSent(Purchase::Server::PaymentSentContext context) {

}

} // namespace swv

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

#pragma once

#include <kj/async.h>

#include <capnp/purchase.capnp.h>

#include <functional>
#include <vector>
#include <map>

namespace swv {

class Purchase : public ::Purchase::Server
{
public:
    struct Price {
        uint64_t coinId;
        int64_t amount;
        kj::String payAddress;

        Price(uint64_t coinId = 0, int64_t amount = 0, kj::String payAddress = kj::String())
            : coinId(coinId),
              amount(amount),
              payAddress(kj::mv(payAddress))
        {}
    };

    Purchase(int64_t votePrice, uint64_t voteId, std::function<void()> onPurchasedCallback,
             std::map<std::string, int64_t> adjustments = {})
        : votePrice(votePrice),
          voteId(voteId),
          adjustments(kj::mv(adjustments)),
          callback(onPurchasedCallback)
    {}
    virtual ~Purchase();

protected:
    // Purchase::Server interface
    virtual ::kj::Promise<void> complete(CompleteContext context);
    virtual ::kj::Promise<void> prices(PricesContext context);
    virtual ::kj::Promise<void> subscribe(SubscribeContext context);
    virtual ::kj::Promise<void> paymentSent(PaymentSentContext);

    void sendNotification();

    bool isComplete = false;
    int64_t votePrice;
    uint64_t voteId;
    std::map<std::string, int64_t> adjustments;
    kj::Maybe<Notifier<capnp::Text>::Client> completionNotifier;
    std::function<void()> callback;
};

} // namespace swv

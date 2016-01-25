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

#include "PurchaseImpl.hpp"

#include <kj/debug.h>

::kj::Promise<void> PurchaseImpl::prices(Purchase::Server::PricesContext context)
{
    auto params = context.getParams();

    auto resultPrices = context.getResults().initPrices(m_prices.size());
    for (unsigned i = 0; i < resultPrices.size(); ++i) {
        resultPrices[i].setCoinId(m_prices[i].coinId);
        resultPrices[i].setAmount(m_prices[i].amount);
        resultPrices[i].setPayAddress(m_prices[i].payAddress.toStdString());
    }
    return kj::READY_NOW;
}

::kj::Promise<void> PurchaseImpl::subscribe(Purchase::Server::SubscribeContext context)
{
    completionNotifier = context.getParams().getNotifier();
    if (isComplete)
        sendNotification();
    return kj::READY_NOW;
}

::kj::Promise<void> PurchaseImpl::paymentSent(Purchase::Server::PaymentSentContext)
{
    isComplete = true;
    sendNotification();
    kj::evalLater(callback).detach([](kj::Exception&& e) {
        KJ_LOG(ERROR, "Error when processing a payment! This is bad.", e);
    });
    return kj::READY_NOW;
}

void PurchaseImpl::sendNotification()
{
    KJ_IF_MAYBE(notifier, completionNotifier) {
        notifier->notifyRequest().setMessage("true");
        notifier->notifyRequest().send();
    }
}

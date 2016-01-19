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

#include <QtCore>

#include <kj/async.h>

#include "capnp/purchase.capnp.h"

#include <functional>

class PurchaseImpl : public Purchase::Server
{
public:
    struct Price {
        quint64 coinId;
        qint64 amount;
        QString payAddress;
    };

    PurchaseImpl(QMap<QString, QList<Price>> promosAndPrices, std::function<void()> onPurchasedCallback)
        : promosAndPrices(promosAndPrices),
          callback(onPurchasedCallback)
    {}
    virtual ~PurchaseImpl(){}

protected:
    // Purchase::Server interface
    virtual ::kj::Promise<void> complete(CompleteContext context);
    virtual ::kj::Promise<void> prices(PricesContext context);
    virtual ::kj::Promise<void> subscribe(SubscribeContext context);
    virtual ::kj::Promise<void> paymentSent(PaymentSentContext);

    void sendNotification();

    bool isComplete = false;
    QMap<QString, QList<Price>> promosAndPrices;
    kj::Maybe<Notifier<capnp::Text>::Client> completionNotifier;
    std::function<void()> callback;
};

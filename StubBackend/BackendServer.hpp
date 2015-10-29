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

#include <backend.capnp.h>

#include <QtCore>

#include <functional>

/**
 * @brief The BackendServer class implements a server for the capnp-defined Backend interface
 */
class BackendServer : public Backend::Server
{
public:
    BackendServer();

    // Backend::Server interface
protected:
    virtual ::kj::Promise<void> increment(IncrementContext context);
    virtual ::kj::Promise<void> listContests(ListContestsContext context);
    virtual ::kj::Promise<void> getContestResults(GetContestResultsContext context);
    virtual ::kj::Promise<void> purchaseResultReport(PurchaseResultReportContext context);
    virtual ::kj::Promise<void> downloadResultReport(DownloadResultReportContext context);
    virtual ::kj::Promise<void> listAvailableResultReports(ListAvailableResultReportsContext context);
    virtual ::kj::Promise<void> purchaseAuditTrail(PurchaseAuditTrailContext context);
    virtual ::kj::Promise<void> downloadAuditTrail(DownloadAuditTrailContext context);
    virtual ::kj::Promise<void> listAvailableAuditTrails(ListAvailableAuditTrailsContext context);

    QMap<quint8, QList<QDateTime>> availableResultReports;
    QMap<quint8, QList<QDateTime>> availableAuditTrails;
};

class ContestResultsImpl : public Backend::ContestResults::Server
{
public:
    ContestResultsImpl(QMap<qint32, qint64> contestResults)
        : contestResults(contestResults)
    {}

    // Backend::ContestResults::Server interface
protected:
    virtual ::kj::Promise<void> results(ResultsContext context);
    virtual ::kj::Promise<void> subscribe(SubscribeContext);

    QMap<qint32, qint64> contestResults;
};

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

protected:
    // Purchase::Server interface
    virtual ::kj::Promise<void> complete(CompleteContext context);
    virtual ::kj::Promise<void> prices(PricesContext context);
    virtual ::kj::Promise<void> subscribe(SubscribeContext context);
    virtual ::kj::Promise<void> paymentSent(PaymentSentContext);

    void sendNotification();

    bool isComplete = false;
    QMap<QString, QList<Price>> promosAndPrices;
    kj::Maybe<Notifier<capnp::AnyPointer>::Client> completionNotifier;
    std::function<void()> callback;
};

#endif // BACKENDSERVER_HPP

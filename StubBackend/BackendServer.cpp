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

#include "BackendServer.hpp"

#include <kj/debug.h>

#include <unistd.h>
#include <iostream>

BackendServer::BackendServer()
{}

::kj::Promise<void> BackendServer::increment(IncrementContext context)
{
    context.getResults().setResult(context.getParams().getNum() + 1);
    return kj::READY_NOW;
}

::kj::Promise<void> BackendServer::listContests(Backend::Server::ListContestsContext context)
{
    auto results = context.getResults();
    auto contests = results.initResults(2);
    contests[0].initContestId(1)[0] = 0;
    contests[0].setVotingStake(10);
    contests[0].setTracksLiveResults(true);

    contests[1].initContestId(1)[0] = 1;
    contests[1].setVotingStake(80000000000);
    contests[1].setTracksLiveResults(false);

    return kj::READY_NOW;
}

::kj::Promise<void> BackendServer::getContestResults(Backend::Server::GetContestResultsContext context)
{
    auto contestId = context.getParams().getContestId()[0];
    auto results = context.getResults();
    switch(contestId) {
    case 0:
        results.setResults(kj::heap<ContestResultsImpl>(QMap<qint32, qint64>({{0,10},{1,88}})));
        break;
    case 1:
        results.setResults(kj::heap<ContestResultsImpl>(QMap<qint32, qint64>({{0,10000},{1,2}})));
        break;
    default:
        KJ_FAIL_REQUIRE("Unknown contest ID.", contestId);
    }

    return kj::READY_NOW;
}

::kj::Promise<void> BackendServer::purchaseResultReport(Backend::Server::PurchaseResultReportContext context)
{
    QMap<QString, QList<PurchaseImpl::Price>> priceList;
    auto params = context.getParams();
    switch(params.getContestId()[0]) {
    case 0:
        priceList[QString::null] = {{0, 10000, "follow-my-vote"},{2, 500, "follow-my-vote"}};
        priceList["FREE"] = {{0, 0, "follow-my-vote"}};
        break;
    case 1:
        priceList[QString::null] = {{0, 30000, "follow-my-vote"}};
        break;
    default:
        KJ_FAIL_REQUIRE("Unknown contest ID.", params.getContestId());
    }

    auto contestId = params.getContestId()[0];
    auto timestamp = QDateTime::fromTime_t(params.getTimestamp());
    auto fulfill = [this, contestId, timestamp] {
        availableResultReports[contestId].append(timestamp);
    };
    context.getResults().setApi(kj::heap<PurchaseImpl>(priceList, fulfill));
    return kj::READY_NOW;
}

::kj::Promise<void> BackendServer::downloadResultReport(Backend::Server::DownloadResultReportContext context)
{
    auto params = context.getParams();
    KJ_REQUIRE(availableResultReports[params.getContestId()[0]].contains(QDateTime::fromTime_t(params.getTimestamp())),
               "Unable to find requested result report.");
    QByteArray report = QString("Result Report for contest ID %1 on %2")
                        .arg(params.getContestId()[0])
                        .arg(QDateTime::fromTime_t(params.getTimestamp()).toString()).toLocal8Bit();
    auto result = context.getResults().initReport(report.size());
    memcpy(result.begin(), report.data(), result.size());
    return kj::READY_NOW;
}

::kj::Promise<void> BackendServer::listAvailableResultReports(Backend::Server::ListAvailableResultReportsContext context)
{
    auto contestId = context.getParams().getContestId()[0];
    auto reports = availableResultReports[contestId];
    auto results = context.getResults().initReports(reports.size());
    for (unsigned i = 0; i < results.size(); ++i)
        results.set(i, reports[i].toTime_t());
    return kj::READY_NOW;
}

::kj::Promise<void> BackendServer::purchaseAuditTrail(Backend::Server::PurchaseAuditTrailContext context)
{
    QMap<QString, QList<PurchaseImpl::Price>> priceList;
    auto params = context.getParams();
    switch(params.getContestId()[0]) {
    case 0:
        priceList[QString::null] = {{0, 40000, "follow-my-vote"},{2, 2000, "follow-my-vote"}};
        break;
    case 1:
        priceList[QString::null] = {{0, 120000, "follow-my-vote"}};
        break;
    default:
        KJ_FAIL_REQUIRE("Unknown contest ID.", params.getContestId());
    }

    auto contestId = params.getContestId()[0];
    auto timestamp = QDateTime::fromTime_t(params.getTimestamp());
    auto fulfill = [this, contestId, timestamp] {
        availableAuditTrails[contestId].append(timestamp);
    };
    context.getResults().setApi(kj::heap<PurchaseImpl>(priceList, fulfill));
    return kj::READY_NOW;
}

::kj::Promise<void> BackendServer::downloadAuditTrail(Backend::Server::DownloadAuditTrailContext context)
{
    auto params = context.getParams();
    KJ_REQUIRE(availableAuditTrails[params.getContestId()[0]].contains(QDateTime::fromTime_t(params.getTimestamp())),
               "Unable to find requested audit trail.");
    QByteArray report = QString("Audit Trail for contest ID %1 on %2")
                        .arg(params.getContestId()[0])
                        .arg(QDateTime::fromTime_t(params.getTimestamp()).toString()).toLocal8Bit();
    auto result = context.getResults().initReport(report.size());
    memcpy(result.begin(), report.data(), result.size());
    return kj::READY_NOW;
}

::kj::Promise<void> BackendServer::listAvailableAuditTrails(Backend::Server::ListAvailableAuditTrailsContext context)
{
    auto contestId = context.getParams().getContestId()[0];
    auto reports = availableAuditTrails[contestId];
    auto results = context.getResults().initReports(reports.size());
    for (unsigned i = 0; i < results.size(); ++i)
        results.set(i, reports[i].toTime_t());
    return kj::READY_NOW;
}

::kj::Promise<void> ContestResultsImpl::results(Backend::ContestResults::Server::ResultsContext context)
{
    auto results = context.getResults().initResults(contestResults.size());
    auto mapResults = contestResults;
    for (unsigned i = 0; i < results.size(); ++i) {
        results[i].setContestant(mapResults.firstKey());
        results[i].setTally(mapResults.take(mapResults.firstKey()));
    }
    return kj::READY_NOW;
}

::kj::Promise<void> ContestResultsImpl::subscribe(Backend::ContestResults::Server::SubscribeContext)
{
    // For now, results never update in the stub server.
    return kj::READY_NOW;
}

::kj::Promise<void> PurchaseImpl::complete(Purchase::Server::CompleteContext context)
{
    context.getResults().setResult(isComplete);
    return kj::READY_NOW;
}

::kj::Promise<void> PurchaseImpl::prices(Purchase::Server::PricesContext context)
{
    auto params = context.getParams();
    QList<Price> prices;
    if (params.hasPromoCode()) {
        QString promoCode = QString::fromStdString(params.getPromoCode());
        KJ_REQUIRE(promosAndPrices.contains(promoCode),
                   "Unknown promo code", params.getPromoCode());
        prices = promosAndPrices[QString::fromStdString(params.getPromoCode())];
    } else {
        KJ_REQUIRE(promosAndPrices.contains(QString::null), "A promo code is required to make this purchase.");
        prices = promosAndPrices[QString::null];
    }

    auto resultPrices = context.getResults().initPrices(prices.size());
    for (unsigned i = 0; i < resultPrices.size(); ++i) {
        resultPrices[i].setCoinId(prices[i].coinId);
        resultPrices[i].setAmount(prices[i].amount);
        resultPrices[i].setPayAddress(prices[i].payAddress.toStdString());
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
        notifier->notifyRequest().send();
    }
}

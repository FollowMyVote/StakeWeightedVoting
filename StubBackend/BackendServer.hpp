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
    virtual ~BackendServer(){}

    // Backend::Server interface
protected:
    virtual ::kj::Promise<void> getContestGenerator(GetContestGeneratorContext context);
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
    virtual ~ContestResultsImpl(){}

    // Backend::ContestResults::Server interface
protected:
    virtual ::kj::Promise<void> results(ResultsContext context);
    virtual ::kj::Promise<void> subscribe(SubscribeContext);

    QMap<qint32, qint64> contestResults;
};

#endif // BACKENDSERVER_HPP

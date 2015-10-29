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

#include "BackendWrapper.hpp"
#include "PromiseWrapper.hpp"

#include <Promise.hpp>

#include <kj/debug.h>

#include <QDebug>
#include <QtQml>
#include <QVariant>
#include <QJsonArray>

namespace swv {

QByteArray convert(capnp::Data::Reader data) {
    return QByteArray((const char*)data.begin(), data.size());
}

BackendWrapper::BackendWrapper(Backend::Client backend, PromiseConverter& promiseWrapper, QObject *parent)
    : QObject(parent),
      promiseWrapper(promiseWrapper),
      backend(kj::mv(backend))
{}

Promise* BackendWrapper::increment(qint8 num)
{
    auto request = backend.incrementRequest();
    request.setNum(num);
    return promiseWrapper.wrap(request.send(), [](Backend::IncrementResults::Reader results) -> QVariantList {
        return {results.getResult()};
    });
}

Promise* BackendWrapper::contestList()
{
    return promiseWrapper.wrap(backend.listContestsRequest().send(),
                [](Backend::ListContestsResults::Reader r) -> QVariantList
    {
        QJsonArray contests;
        for (const Backend::ListedContest::Reader& contest : r.getResults())
            contests << QJsonObject{{"contestId", QString(convert(contest.getContestId()).toHex())},
                                    {"votingStake", qint64(contest.getVotingStake())},
                                    {"tracksLiveResults", contest.getTracksLiveResults()}};
        return {contests};
    });
}

} // namespace swv

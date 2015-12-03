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
#include "PromiseConverter.hpp"

#include <Promise.hpp>

#include <kj/debug.h>

#include <QDebug>
#include <QtQml>
#include <QVariant>

namespace swv {

QByteArray convert(capnp::Data::Reader data) {
    return QByteArray(reinterpret_cast<const char*>(data.begin()), static_cast<signed>(data.size()));
}
QVariantMap convert(ContestGenerator::ListedContest::Reader contest) {
    return {{"contestId", QString(convert(contest.getContestId()).toHex())},
            {"votingStake", qint64(contest.getVotingStake())},
            {"tracksLiveResults", contest.getTracksLiveResults()}};
}

kj::ForkedPromise<ContestGenerator::Client> makeGeneratorPromise(Backend::Client backend) {
    return backend.getContestGeneratorRequest().send().then([](capnp::Response<Backend::GetContestGeneratorResults> r) {
        return r.getGenerator();
    }).fork();
}

BackendWrapper::BackendWrapper(Backend::Client backend, PromiseConverter& promiseWrapper, QObject *parent)
    : QObject(parent),
      promiseConverter(promiseWrapper),
      backend(kj::mv(backend)),
      generatorPromise(makeGeneratorPromise(this->backend))
{
}

Promise* BackendWrapper::getContest()
{
    auto promiseForContest = generatorPromise.addBranch().then([](ContestGenerator::Client generator) {
        return generator.getContestRequest().send().then([](capnp::Response<ContestGenerator::GetContestResults> r) {
            return r;
        });
    });
    return promiseConverter.convert(kj::mv(promiseForContest),
                                 [](ContestGenerator::GetContestResults::Reader r) -> QVariantList {
        return {convert(r.getNextContest())};
    });
}

Promise* BackendWrapper::getContests(int count)
{
    qDebug() << "Requesting" << count << "contests";
    auto promiseForContest = generatorPromise.addBranch().then([count](ContestGenerator::Client generator) {
        auto request = generator.getContestsRequest();
        request.setCount(count);
        return request.send().then([](capnp::Response<ContestGenerator::GetContestsResults> r) { return r; });
    });
    return promiseConverter.convert(kj::mv(promiseForContest),
                                 [](capnp::Response<ContestGenerator::GetContestsResults> r) -> QVariantList {
        qDebug() << "Got" << r.getNextContests().size() << "contests";
        QVariantList contests;
        for (auto contest : r.getNextContests())
            contests.append(convert(contest));
        return {contests};
    });
}

void BackendWrapper::refreshContests()
{
    generatorPromise = makeGeneratorPromise(backend);
}

} // namespace swv

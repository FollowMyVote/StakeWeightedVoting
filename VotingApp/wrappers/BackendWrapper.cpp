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
QJsonObject convert(ContestGenerator::ListedContest::Reader contest) {
    return {{"contestId", QString(convert(contest.getContestId()).toHex())},
                         {"votingStake", qint64(contest.getVotingStake())},
                         {"tracksLiveResults", contest.getTracksLiveResults()}};
}

kj::ForkedPromise<ContestGenerator::Client> makeGeneratorPromise(Backend::Client backend) {
    // The spurious + here converts the lambda to a function pointer (inefficient!) because otherwise the stupid
    // can't figure out whether to call the lambda directly or cast it to a function pointer and call that. Obviously
    // it should just call them lambda, but I can't figure out a way to explicitly state "do the obviously correct
    // thing," so for now I'm explicitly stating "do the obviously incorrect thing that works anyways, but slower."
    return backend.getContestGeneratorRequest().send().then(+[](Backend::GetContestGeneratorResults::Reader r) {
        return r.getGenerator();
    }).fork();
}

BackendWrapper::BackendWrapper(Backend::Client backend, PromiseConverter& promiseWrapper, QObject *parent)
    : QObject(parent),
      promiseWrapper(promiseWrapper),
      backend(kj::mv(backend)),
      generatorPromise(makeGeneratorPromise(this->backend))
{
}

Promise* BackendWrapper::increment(quint8 num)
{
    auto request = backend.incrementRequest();
    request.setNum(num);
    return promiseWrapper.wrap(request.send(), [](Backend::IncrementResults::Reader results) -> QVariantList {
        return {results.getResult()};
    });
}

Promise* BackendWrapper::getContest()
{
    auto promiseForContest = generatorPromise.addBranch().then([](ContestGenerator::Client generator) {
        return generator.nextRequest().send().then(+[](ContestGenerator::NextResults::Reader r) { return r; });
    });
    return promiseWrapper.wrap(kj::mv(promiseForContest), [](ContestGenerator::NextResults::Reader r) -> QVariantList {
        return {convert(r.getNextContest())};
    });
}

Promise* BackendWrapper::getContests(int count)
{
    auto promiseForContest = generatorPromise.addBranch().then([count](ContestGenerator::Client generator) {
        auto request = generator.nextCountRequest();
        request.setCount(count);
        return request.send().then(+[](ContestGenerator::NextCountResults::Reader r) { return r; });
    });
    return promiseWrapper.wrap(kj::mv(promiseForContest),
                               [](ContestGenerator::NextCountResults::Reader r) -> QVariantList {
        QJsonArray contests;
        for (auto contest : r.getNextContests())
            contests.append(convert(contest));
        return {contests};
    });
}

} // namespace swv

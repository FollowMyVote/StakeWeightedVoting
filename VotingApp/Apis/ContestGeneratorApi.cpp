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

#include "ContestGeneratorApi.hpp"
#include "ContestResultsApi.hpp"
#include "Converters.hpp"
#include "DataStructures/Contest.hpp"

#include <ids.capnp.h>

#include <kj/debug.h>

namespace swv {

ContestGeneratorApi::ContestGeneratorApi(ContestGenerator::Client generator,
                                         PromiseConverter& converter,
                                         QObject *parent)
    : QObject(parent),
      generator(generator),
      converter(converter)
{}

ContestGeneratorApi::~ContestGeneratorApi() noexcept
{}

QJSValue ContestGeneratorApi::getContests(int count) {
    KJ_LOG(DBG, "Requesting contests", count);
    auto request = generator.getValuesRequest();
    request.setCount(count);
    m_isFetchingContests = true;
    emit this->isFetchingContestsChanged(true);

    auto pass = [this](auto&& e) {
        m_isFetchingContests = false;
        emit this->isFetchingContestsChanged(false);
        return kj::mv(e);
    };

    auto contestsPromise = request.send().then([this, count](capnp::Response<ContestGenerator::GetValuesResults> r) {
        if (!r.hasValues() || r.getValues().size() < count) {
            m_isOutOfContests = true;
            emit this->isOutOfContestsChanged(true);
        }
        m_isFetchingContests = false;
        emit this->isFetchingContestsChanged(false);
        return kj::mv(r);
    }, pass);

    return converter.convert(kj::mv(contestsPromise),
                             [this](capnp::Response<ContestGenerator::GetValuesResults> r) -> QVariant {
        KJ_LOG(DBG, "Got contests", r.toString());
        QVariantList contests;
        for (auto contestWrapper : r.getValues()) {
            auto contest = contestWrapper.getValue();
            auto results = new ContestResultsApi(contest.getContestResults(), converter);
            QQmlEngine::setObjectOwnership(results, QQmlEngine::JavaScriptOwnership);
            // TODO: Set engagement notification API (when server defines it)
            contests.append(
                        QVariantMap{
                            {"contestId", QString(convertBlob(ReaderPacker(contest.getContestId()).array()).toHex())},
                            {"votingStake", qint64(contest.getVotingStake())},
                            {"resultsApi", QVariant::fromValue(results)}
                        });
        }
        return {QVariant(contests)};
    });
}

}

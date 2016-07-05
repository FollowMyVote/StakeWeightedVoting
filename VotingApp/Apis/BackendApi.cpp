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

#include "BackendApi.hpp"
#include "ContestResultsApi.hpp"
#include "PromiseConverter.hpp"
#include "Apis/ContestGeneratorApi.hpp"
#include "Apis/PurchaseContestRequestApi.hpp"
#include "Apis/ContestCreatorApi.hpp"
#include "Converters.hpp"

#include <Promise.hpp>

#include <kj/debug.h>

#include <QDebug>
#include <QtQml>
#include <QVariant>
#include <Utilities.hpp>

namespace swv {

BackendApi::BackendApi(Backend::Client backend, PromiseConverter& promiseConverter, QObject *parent)
    : QObject(parent),
      promiseConverter(promiseConverter),
      m_backend(kj::mv(backend))
{}

BackendApi::~BackendApi() noexcept
{}

ContestGeneratorApi* BackendApi::getFeedGenerator() {
    return new ContestGeneratorApi(m_backend.getContestFeedRequest().send().getGenerator(), promiseConverter);
}

ContestGeneratorApi* BackendApi::getContestsByCreator(QString creator) {
    KJ_LOG(DBG, "Getting contests by creator", creator.toStdString());
    auto request = m_backend.searchContestsRequest();
    auto filters = request.initFilters(1);
    filters[0].setType(Backend::Filter::Type::CONTEST_CREATOR);
    auto arguments = filters[0].initArguments(1);
    arguments.set(0, creator.toStdString());

    return new ContestGeneratorApi(request.send().getGenerator(), promiseConverter);
}

ContestGeneratorApi* BackendApi::getContestsByCoin(quint64 coinId) {
    KJ_LOG(DBG, "Getting contests by coin", coinId);
    auto request = m_backend.searchContestsRequest();
    auto filters = request.initFilters(1);
    filters[0].setType(Backend::Filter::Type::CONTEST_COIN);
    auto arguments = filters[0].initArguments(1);
    arguments.set(0, std::to_string(coinId));

    return new ContestGeneratorApi(request.send().getGenerator(), promiseConverter);
}

ContestGeneratorApi*BackendApi::getVotedContests() {
    KJ_LOG(DBG, "Getting contests by voter");
    auto request = m_backend.searchContestsRequest();
    auto filters = request.initFilters(1);
    filters[0].setType(Backend::Filter::Type::CONTEST_VOTER);

    return new ContestGeneratorApi(request.send().getGenerator(), promiseConverter);
}

ContestResultsApi* BackendApi::getContestResults(QString contestId) {
    auto request = m_backend.getContestResultsRequest();
    auto blob = QByteArray::fromHex(contestId.toLocal8Bit());
    BlobMessageReader reader(convertBlob(blob));
    request.setContestId(reader->getRoot<::ContestId>());

    return new ContestResultsApi(request.send().getResults());
}

ContestCreatorApi* BackendApi::contestCreator() {
    // Lazy load the creator; most runs we will probably never need it.
    if (creator.get() == nullptr)
        creator = kj::heap<ContestCreatorApi>(m_backend.createContestRequest().send().getCreator());
    return creator.get();
}

} // namespace swv

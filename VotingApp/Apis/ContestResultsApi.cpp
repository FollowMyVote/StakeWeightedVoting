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

#include "ContestResultsApi.hpp"
#include "Converters.hpp"
#include "DecisionGeneratorApi.hpp"
#include "DataStructures/Contest.hpp"

#include <QBarSet>
#include <QDebug>

namespace swv {

void ContestResultsApi::updateResults(capnp::List<ContestResults::TalliedOpinion>::Reader talliedOpinions) {
    // Allocate space in m_contestantResults
    m_contestantResults.clear();
    for (auto tally : talliedOpinions)
        while (tally.getContestant().isContestant() &&
               m_contestantResults.size() <= tally.getContestant().getContestant())
            m_contestantResults.append(0);

    for (auto contestantTally : talliedOpinions) {
        auto tally = QVariant(static_cast<qreal>(contestantTally.getTally()));
        if (contestantTally.getContestant().isContestant())
            m_contestantResults[contestantTally.getContestant().getContestant()] = tally;
        else
            m_writeInResults.insert(convertText(contestantTally.getContestant().getWriteIn()), tally);
    }
    emit resultsChanged();
}

ContestResultsApi::ContestResultsApi(ContestResults::Client resultsApi, PromiseConverter& converter)
    : resultsApi(resultsApi),
      converter(converter) {
    auto resultsRequest = resultsApi.resultsRequest().send();

    converter.adopt(resultsRequest.then([this](capnp::Response<ContestResults::ResultsResults> results) {
        auto talliedOpinions = results.getResults();
        updateResults(talliedOpinions);
    }));

    auto subscribeRequest = resultsApi.subscribeRequest();
    subscribeRequest.setNotifier(kj::heap<ResultsNotifier>(*this));
    converter.adopt(subscribeRequest.send());
}

ContestResultsApi::~ContestResultsApi() noexcept {}

DecisionGeneratorApi* ContestResultsApi::getDecisionGenerator() {
    return new DecisionGeneratorApi(resultsApi.decisionsRequest().send().getDecisionGenerator(), converter);
}

ContestResultsApi::ResultsNotifier::ResultsNotifier(ContestResultsApi& resultsApi)
    : resultsApi(resultsApi) {}

::kj::Promise<void> ContestResultsApi::ResultsNotifier::notify(NotifyContext context) {
    auto talliedOpinions = context.getParams().getNotification();
    resultsApi.updateResults(talliedOpinions);
    return kj::READY_NOW;
}

} // namespace swv

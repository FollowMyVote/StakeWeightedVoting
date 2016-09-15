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

#ifndef CONTESTRESULTSAPI_HPP
#define CONTESTRESULTSAPI_HPP

#include <contestgenerator.capnp.h>

#include <PromiseConverter.hpp>

#include <QObject>
#include <QBarSeries>
#include <QValueAxis>
#include <QBarCategoryAxis>

#include <kj/debug.h>

namespace swv {
class DecisionGeneratorApi;
namespace data { class Contest; }

class ContestResultsApi : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList contestantResults READ contestantResults NOTIFY resultsChanged)
    Q_PROPERTY(QVariantMap writeInResults READ writeInResults NOTIFY resultsChanged)

    ContestResults::Client resultsApi;
    PromiseConverter& converter;

    class ResultsNotifier : public Notifier<capnp::List<ContestResults::TalliedOpinion>>::Server {
        ContestResultsApi& resultsApi;

    public:
        ResultsNotifier(ContestResultsApi& resultsApi);
        virtual ~ResultsNotifier(){}

    protected:
        virtual ::kj::Promise<void> notify(NotifyContext context) override;
    };

    void updateResults(capnp::List<ContestResults::TalliedOpinion>::Reader talliedOpinions);

    QVariantList m_contestantResults;
    QVariantMap m_writeInResults;

public:
    ContestResultsApi(ContestResults::Client resultsApi, PromiseConverter& converter);
    virtual ~ContestResultsApi() noexcept;

    QVariantList contestantResults() const { return m_contestantResults; }
    QVariantMap writeInResults() const { return m_writeInResults; }

    /// Request and return a new decision generator. Caller takes ownership.
    Q_INVOKABLE swv::DecisionGeneratorApi* getDecisionGenerator();

signals:
    void resultsChanged();
};

} // namespace swv
#endif // CONTESTRESULTSAPI_HPP

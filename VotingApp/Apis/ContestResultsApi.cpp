#include "ContestResultsApi.hpp"
#include "Converters.hpp"
#include "DataStructures/Contest.hpp"

#include <QBarSet>
#include <QDebug>

namespace swv {

void ContestResultsApi::updateBarSeries(capnp::List<Backend::ContestResults::TalliedOpinion>::Reader talliedOpinions) {
    if (m_results == nullptr)
        return;

    std::vector<std::pair<QString, int64_t>> writeInResults;
    std::vector<int64_t> contestantResults(contest.get_contestants().size());

    for (auto tally : talliedOpinions) {
        if (tally.getContestant().isContestant())
            contestantResults[tally.getContestant().getContestant()] = tally.getTally();
        else
            writeInResults.emplace_back(convertText(tally.getContestant().getWriteIn()), tally.getTally());
    }

    // Sort write-ins in decreasing order of votes
    std::sort(writeInResults.begin(), writeInResults.end(), [](auto a, auto b) { return a.second > b.second; });

    auto barSet = new QtCharts::QBarSet("Contestant Tallies", m_results);
    qreal maxVotes = 0;
    for (auto result : contestantResults) {
        barSet->append(result);
        maxVotes = std::max<qreal>(maxVotes, result);
    }
    for (auto writeIn : writeInResults) {
        barSet->append(writeIn.second);
        maxVotes = std::max<qreal>(maxVotes, writeIn.second);
    }
    m_results->clear();
    m_results->append(barSet);

    m_yAxis->setMin(0);
    m_yAxis->setMax(maxVotes * 1.05);
    m_yAxis->applyNiceNumbers();

    auto contestants = contest.get_contestants();
    QStringList contestantNames;
    std::transform(contestants.begin(), contestants.end(), std::back_inserter(contestantNames),
                   [](const QVariant& v) { return v.toMap()["name"].toString(); });
    std::transform(writeInResults.begin(), writeInResults.end(), std::back_inserter(contestantNames),
                   [](auto pair) { return pair.first; });
    m_xAxis->setCategories(kj::mv(contestantNames));
}

ContestResultsApi::ContestResultsApi(Backend::ContestResults::Client resultsApi, const data::Contest& contest)
    : resultsApi(resultsApi),
      contest(contest),
      m_tasks(errorHandler) {
    auto resultsRequest = resultsApi.resultsRequest().send();

    m_tasks.add(resultsRequest.then([this](capnp::Response<Backend::ContestResults::ResultsResults> results) {
        auto talliedOpinions = results.getResults();
        updateBarSeries(talliedOpinions);
    }));

    auto subscribeRequest = resultsApi.subscribeRequest();
    subscribeRequest.setNotifier(kj::heap<ResultsNotifier>(*this));
    m_tasks.add(subscribeRequest.send().then([](auto){}));
}

ContestResultsApi::~ContestResultsApi() noexcept {}

void ContestResultsApi::setResults(QtCharts::QBarSeries* results) {
    if (results == m_results)
        return;

    if (results) {
        results->clear();
        auto sets = m_results->barSets();

        // Move all bar sets from old series to new series
        for (auto set : sets) {
            m_results->take(set);
            results->append(set);
        }

        m_results->deleteLater();
    }

    m_results = results;
    emit resultsChanged(results);
}

ContestResultsApi::ResultsNotifier::ResultsNotifier(ContestResultsApi& resultsApi)
    : resultsApi(resultsApi) {}

::kj::Promise<void> ContestResultsApi::ResultsNotifier::notify(NotifyContext context) {
    auto talliedOpinions = context.getParams().getNotification();
    resultsApi.updateBarSeries(talliedOpinions);
    return kj::READY_NOW;
}

void ContestResultsApi::ErrorHandler::taskFailed(kj::Exception&& exception) {
    // TODO: Do something smart in the UI
    KJ_LOG(ERROR, "Exception from RPC", exception);
}

} // namespace swv

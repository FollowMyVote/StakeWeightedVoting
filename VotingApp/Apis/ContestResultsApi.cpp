#include "ContestResultsApi.hpp"
#include "Converters.hpp"
#include "DataStructures/Contest.hpp"

#include <QBarSet>


namespace swv {

void ContestResultsApi::updateBarSeries(capnp::List<Backend::ContestResults::TalliedOpinion>::Reader talliedOpinions) {
    std::map<QString, int64_t> intermediateResults;

    // Convert talliedOpinions to a more accessible format, which refers to all contestants by name
    for (auto tally : talliedOpinions) {
        auto contestant = tally.getContestant().isContestant()?
                              contest.get_contestants()[tally.getContestant().getContestant()].toString()
                            : convertText(tally.getContestant().getWriteIn());
        intermediateResults[contestant] = tally.getTally();
    }

    // Update all the tallies already in the results, removing them from intermediateResults as we go
    for (auto set : results()->barSets()) {
        if (set->count() != 1) {
            // Either set is empty, or has multiple values. This ensures the set is empty, then adds a single value.
            set->remove(0, set->count());
            set->append(0);
        }

        if (intermediateResults.count(set->label())) {
            set->replace(0, intermediateResults[set->label()]);
            intermediateResults.erase(set->label());
        }
    }
    // Add any tallies that were not in the results yet
    for (auto result : intermediateResults) {
        auto set = new QtCharts::QBarSet(result.first, m_results);
        set->append(result.second);
        m_results->append(set);
    }

    // Sort the candidates by votes
    auto sets = m_results->barSets();
    std::sort(sets.begin(), sets.end(), [](const QtCharts::QBarSet* a, const QtCharts::QBarSet* b) {
        // Sort by vote tally, from greatest to least
        if (!qFuzzyCompare(1 + a->at(0), 1 + b->at(0)))
            return a->at(0) > b->at(0);
        // Break ties with contestant name, from least to greatest
        return a->label() < b->label();
    });
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
    if (results == m_results || results == nullptr)
        return;

    results->clear();
    auto sets = m_results->barSets();

    // Move all bar sets from old series to new series
    for (auto set : sets) {
        m_results->take(set);
        results->append(set);
    }

    m_results->deleteLater();
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

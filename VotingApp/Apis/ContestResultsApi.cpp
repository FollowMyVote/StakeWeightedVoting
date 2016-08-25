#include "ContestResultsApi.hpp"
#include "Converters.hpp"
#include "DataStructures/Contest.hpp"

#include <QBarSet>
#include <QDebug>

namespace swv {

void ContestResultsApi::updateResults(capnp::List<ContestResults::TalliedOpinion>::Reader talliedOpinions) {
    // Allocate space in m_contestantResults
    m_contestantResults.clear();
    for (auto tally : talliedOpinions)
        if (tally.getContestant().isContestant())
            m_contestantResults.append(0);

    for (auto tally : talliedOpinions) {
        if (tally.getContestant().isContestant())
            m_contestantResults[tally.getContestant().getContestant()] = tally.getTally();
        else
            m_writeInResults.insert(convertText(tally.getContestant().getWriteIn()), tally.getTally());
    }
    emit resultsChanged();
}

ContestResultsApi::ContestResultsApi(ContestResults::Client resultsApi)
    : resultsApi(resultsApi),
      m_tasks(errorHandler) {
    auto resultsRequest = resultsApi.resultsRequest().send();

    m_tasks.add(resultsRequest.then([this](capnp::Response<ContestResults::ResultsResults> results) {
        auto talliedOpinions = results.getResults();
        updateResults(talliedOpinions);
    }));

    auto subscribeRequest = resultsApi.subscribeRequest();
    subscribeRequest.setNotifier(kj::heap<ResultsNotifier>(*this));
    m_tasks.add(subscribeRequest.send().then([](auto){}));
}

ContestResultsApi::~ContestResultsApi() noexcept {}

ContestResultsApi::ResultsNotifier::ResultsNotifier(ContestResultsApi& resultsApi)
    : resultsApi(resultsApi) {}

::kj::Promise<void> ContestResultsApi::ResultsNotifier::notify(NotifyContext context) {
    auto talliedOpinions = context.getParams().getNotification();
    resultsApi.updateResults(talliedOpinions);
    return kj::READY_NOW;
}

void ContestResultsApi::ErrorHandler::taskFailed(kj::Exception&& exception) {
    // TODO: Do something smart in the UI
    KJ_LOG(ERROR, "Exception from RPC", exception);
}

} // namespace swv

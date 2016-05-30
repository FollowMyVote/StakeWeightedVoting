#include "ContestResultsApi.hpp"

namespace swv {

void ContestResultsApi::updateBarSeries(capnp::List<Backend::ContestResults::TalliedOpinion>::Reader talliedOpinions) {
    m_results.clear();
    for (auto tally : talliedOpinions) {
        // TODO: convert each opinion to QBarSeries
    }
    emit resultsChanged(&m_results);
}

ContestResultsApi::ContestResultsApi(Backend::ContestResults::Client resultsApi)
    : resultsApi(resultsApi),
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

ContestResultsApi::ResultsNotifier::ResultsNotifier(ContestResultsApi& resultsApi)
    : resultsApi(resultsApi) {}

::kj::Promise<void> ContestResultsApi::ResultsNotifier::notify(NotifyContext context) {
    auto talliedOpinions = context.getParams().getNotification();
    resultsApi.updateBarSeries(talliedOpinions);
}

void ContestResultsApi::ErrorHandler::taskFailed(kj::Exception&& exception) {
    // TODO: Do something smart in the UI
    KJ_LOG(ERROR, "Exception from RPC", exception);
}

} // namespace swv

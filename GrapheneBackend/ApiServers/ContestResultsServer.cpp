#include "ContestResultsServer.hpp"
#include "VoteDatabase.hpp"

#include <kj/debug.h>

namespace swv {

ContestResultsServer::ContestResultsServer(VoteDatabase& vdb, gch::operation_history_id_type contestId)
    : vdb(vdb), contestId(contestId) {}

::kj::Promise<void> ContestResultsServer::results(ContestResults::Server::ResultsContext context) {
    KJ_LOG(DBG, __FUNCTION__, context.getParams());
    auto results = tallyResults();
    auto resultCount = results.size();
    populateResults(context.getResults().initResults(resultCount),
                    kj::mv(results));

    return kj::READY_NOW;
}

::kj::Promise<void> ContestResultsServer::subscribe(ContestResults::Server::SubscribeContext context) {
    KJ_LOG(DBG, __FUNCTION__, context.getParams());
    // TODO: Consider using database::changed_objects signal instead of a secondary index and vdb.contestResultsUpdated
    notifiers.emplace_back(context.getParams().getNotifier());
    subscriptions.emplace_back(vdb.contestResultsUpdated.connect(
                             [this] (gch::operation_history_id_type contestId) mutable -> void {
                                   try {
                                       if (contestId == this->contestId) {
                                           for (auto& notifier : notifiers) {
                                               auto request = notifier.notifyRequest();
                                               auto results = tallyResults();
                                               auto resultCount = results.size();
                                               populateResults(request.initNotification(resultCount),
                                                               kj::mv(results));
                                               request.send();
                                           }
                                       }
                                   } catch (kj::Exception& e) {
                                       KJ_LOG(ERROR, "Exception in results notifier", e);
                                   }
                               }));

    return kj::READY_NOW;
}

const Contest& ContestResultsServer::getContest() {
    auto& index = vdb.contestIndex().indices().get<ById>();
    auto itr = index.find(contestId);
    KJ_ASSERT(itr != index.end(), "No contest with the specified ID was found.");
    return *itr;
}

ContestResultsServer::Results ContestResultsServer::tallyResults() {
    // TODO: implement me
    Results results;
}

void ContestResultsServer::populateResults(capnp::List<ContestResults::TalliedOpinion>::Builder resultsBuilder,
                                           Results results) {
    // TODO: implement me
}

} // namespace swv

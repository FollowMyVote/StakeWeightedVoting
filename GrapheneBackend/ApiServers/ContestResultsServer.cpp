#include "ContestResultsServer.hpp"
#include "VoteDatabase.hpp"

#include <kj/debug.h>

namespace swv {

ContestResultsServer::ContestResultsServer(VoteDatabase& vdb, gch::operation_history_id_type contestId)
    : vdb(vdb), contestId(contestId) {}

::kj::Promise<void> ContestResultsServer::results(Backend::ContestResults::Server::ResultsContext context) {
    KJ_LOG(DBG, __FUNCTION__, context.getParams());
    const auto& contest = getContest();
    auto results = context.initResults().initResults(contest.contestantResults.size() + contest.writeInResults.size());
    populateResults(results, contest);

    return kj::READY_NOW;
}

::kj::Promise<void> ContestResultsServer::subscribe(Backend::ContestResults::Server::SubscribeContext context) {
    KJ_LOG(DBG, __FUNCTION__, context.getParams());
    // TODO: Consider using database::changed_objects signal instead of a secondary index and vdb.contestResultsUpdated
    notifiers.emplace_back(context.getParams().getNotifier());
    subscriptions.emplace_back(vdb.contestResultsUpdated.connect(
                             [this] (gch::operation_history_id_type contestId) mutable -> void {
                                   if (contestId == this->contestId) {
                                       for (auto& notifier : notifiers) {
                                           auto request = notifier.notifyRequest();
                                           populateResults(request.getNotification(), getContest());
                                           request.send();
                                       }
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

void ContestResultsServer::populateResults(capnp::List<Backend::ContestResults::TalliedOpinion>::Builder results,
                                           const Contest& contest) {
    auto resultIndex = 0u;
    for (const auto& contestantResult : contest.contestantResults) {
        auto result = results[resultIndex++];
        result.initContestant().setContestant(contestantResult.first);
        result.setTally(contestantResult.second);
    }
    for (const auto& writeInResult : contest.writeInResults) {
        auto result = results[resultIndex++];
        result.initContestant().setWriteIn(writeInResult.first);
        result.setTally(writeInResult.second);
    }
}

} // namespace swv

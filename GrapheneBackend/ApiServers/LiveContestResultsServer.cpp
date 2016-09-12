#include "LiveContestResultsServer.hpp"
#include "SnapshotDecisionGenerator.hpp"
#include "VoteDatabase.hpp"

#include <kj/debug.h>

#include <iostream>

namespace swv {

LiveContestResultsServer::LiveContestResultsServer(VoteDatabase& vdb, gch::operation_history_id_type contestId)
    : vdb(vdb), contestId(contestId) {}

int64_t LiveContestResultsServer::totalVotingStake() {
    auto results = tallyResults();
    return std::accumulate(results.begin(), results.end(), gch::share_type(),
                           [](gch::share_type sum, const auto& result) {
        return sum + result.second;
    }).value;
}

::kj::Promise<void> LiveContestResultsServer::results(ContestResults::Server::ResultsContext context) {
    KJ_LOG(DBG, __FUNCTION__, context.getParams());
    try {
        auto results = tallyResults();
        auto resultCount = results.size();
        populateResults(context.getResults().initResults(resultCount), kj::mv(results));
    } catch (fc::exception& e) {
        edump((e.to_detail_string()));
    }

    return kj::READY_NOW;
}

::kj::Promise<void> LiveContestResultsServer::subscribe(ContestResults::Server::SubscribeContext context) {
    KJ_LOG(DBG, __FUNCTION__, context.getParams());
    try {
        notifiers.emplace_back(context.getParams().getNotifier());
        subscriptions.emplace_back(vdb.db().applied_block.connect([this] (const gch::signed_block&) mutable -> void {
            try {
                for (auto& notifier : notifiers) {
                    auto request = notifier.notifyRequest();
                    auto results = tallyResults();
                    auto resultCount = results.size();
                    populateResults(request.initNotification(resultCount),
                                    kj::mv(results));
                    request.send();
                }
            } catch (kj::Exception& e) {
                KJ_LOG(ERROR, "Exception in results notifier", e);
            }
        }));
    } catch (fc::exception& e) {
        edump((e.to_detail_string()));
    }

    return kj::READY_NOW;
}

::kj::Promise<void> LiveContestResultsServer::decisions(ContestResults::Server::DecisionsContext context) {
    KJ_LOG(DBG, __FUNCTION__, context.getParams());
    context.getResults().setDecisionGenerator(kj::heap<SnapshotDecisionGenerator>(contestId, vdb));
    return kj::READY_NOW;
}

const Contest& LiveContestResultsServer::getContest() {
    auto& index = vdb.contestIndex().indices().get<ById>();
    auto itr = index.find(contestId);
    KJ_REQUIRE(itr != index.end(), "No contest with the specified ID was found.");
    return *itr;
}

LiveContestResultsServer::Results LiveContestResultsServer::tallyResults() {
    Results results;

    /*
     * We need to tally the results on the contest. The decisions-by-contest index orders decisions by contest, then by
     * voter, then by order of creation. We need to examine the most recent decision each voter who has voted on this
     * contest has published. So the algorithm is:
     *
     * Let C = the contest being tallied
     * For each voter V who has cast any decisions on C:
     *   Let D = last decision V cast on C
     *   Add D to tally
     */

    const auto& contest = getContest();
    auto contestantCount = contest.contestants.size();
    auto& index = vdb.decisionIndex().indices().get<ByContest>();
    // Find the first decision on this contest and get its voter
    auto begin = index.lower_bound(boost::make_tuple(contestId));

    // begin now points to the first decision in a block of decisions cast by voter on contest

#define EXPECT(condition, ...) \
    if (!(condition)) { \
        KJ_LOG(ERROR, __VA_ARGS__); \
    }

    // While we haven't processed all decisions in the index, and we haven't moved into decisions on the next contest
    while (begin != index.end() && begin->contestId == contestId) {
        auto voter = begin->voter;
        auto stake = voter(vdb.db()).balance;
        // Find the end of the block of decisions by this voter on this contest
        auto end = index.upper_bound(boost::make_tuple(contestId, voter));
        // Point begin at the next block of decisions, which prepares it for the next iteration of the loop
        begin = end;

        // end points to the first decision in the next block. Decrement it to point to the last decision in the
        // current block. It should be safe to blindly decrement this, since we got the voter from a decision, so it's
        // guaranteed there's at least one decision with the specified voter.
        --end;
        const auto& opinions = end->opinions;

        if (opinions.size() == 0 || opinions.begin()->second == 0)
            // Null decision; there's nothing to tally. Move on to the next voter.
            continue;
        EXPECT(opinions.size() == 1, "Only single-opinion decisions are currently supported",
               fc::json::to_pretty_string(opinions))

        auto opinionIndex = opinions.begin()->first;
        auto opinion = opinions.begin()->second;
        EXPECT(opinion == 1, "Only opinions of 1 are currently supported.", opinion);

        Results::key_type candidate;
        if (opinionIndex < contestantCount) {
            // Normal contestant; just take his index
            candidate = opinionIndex;
        } else {
            // Write-in; look up his name
            auto writeInIndex = opinionIndex - contestantCount;
            EXPECT(writeInIndex < end->writeIns.size(), "Decision references a nonexistent write-in");
            candidate = end->writeIns[writeInIndex].first;
        }

        // Add the voter's stake to the tally for the voter's chosen candidate
        results[candidate] += stake;
    }

#undef EXPECT

    return results;
}

void LiveContestResultsServer::populateResults(capnp::List<ContestResults::TalliedOpinion>::Builder resultsBuilder,
                                           Results results) {
    unsigned index = 0;
    for (const auto& result : results) {
        auto resultBuilder = resultsBuilder[index++];
        resultBuilder.setTally(result.second.value);
        if (result.first.type() == typeid(int32_t))
            resultBuilder.getContestant().setContestant(boost::get<int32_t>(result.first));
        else
            resultBuilder.getContestant().setWriteIn(boost::get<std::string>(result.first));
    }
}

} // namespace swv

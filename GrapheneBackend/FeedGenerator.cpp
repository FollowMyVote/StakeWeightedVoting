#include "FeedGenerator.hpp"

#include <graphene/chain/database.hpp>

#include <kj/vector.h>

#include <numeric>

namespace swv {

capnp::Data::Reader readerOf(const std::vector<char>& data) {
    return kj::ArrayPtr<const kj::byte>((const kj::byte*)data.data(), data.size());
}

void swv::FeedGenerator::populateContest(ContestGenerator::ListedContest::Builder nextContest) {
    auto packedId = fc::raw::pack(currentContest->contestId);
    nextContest.setContestId(readerOf(packedId));
    nextContest.setTracksLiveResults(false);

    // Shorter type names
    using contestantResult = decltype (currentContest->contestantResults)::value_type;
    using writeInResult = decltype (currentContest->writeInResults)::value_type;
    // Cliff notes: votingStake = sum(all votes for contestants) + sum(all votes for write-ins)
    auto votingStake = std::accumulate(currentContest->contestantResults.begin(),
                                       currentContest->contestantResults.end(),
                                       std::pair<int32_t, int64_t>(),
                                       [](const contestantResult& a, const contestantResult& b) -> contestantResult
    { return {0, a.second + b.second}; }).second
                       + std::accumulate(currentContest->writeInResults.begin(), currentContest->writeInResults.end(),
                                         std::pair<std::string, int64_t>(),
                                         [](const writeInResult& a, const writeInResult& b) -> writeInResult
    { return {{}, a.second + b.second}; }).second;
    nextContest.setVotingStake(votingStake);
}

swv::FeedGenerator::FeedGenerator(const Contest* firstContest, const gch::database& db)
    : currentContest(firstContest),
      db(db),
      index(db.get_index_type<ContestIndex>().indices().get<ByStartTime>()) {}

swv::FeedGenerator::~FeedGenerator(){}

::kj::Promise<void> swv::FeedGenerator::getContest(GetContestContext context) {
    if (currentContest == nullptr)
        return kj::READY_NOW;

    auto itr = index.iterator_to(*currentContest);
    if (itr == index.end()) {
        currentContest = nullptr;
        return kj::READY_NOW;
    }
    while (++itr != index.end() && itr->endTime < fc::time_point(db.head_block_time()));
    if (itr == index.end()) {
        currentContest = nullptr;
        return kj::READY_NOW;
    }
    currentContest = &*itr;
    populateContest(context.initResults().initNextContest());

    return kj::READY_NOW;
}

::kj::Promise<void> swv::FeedGenerator::getContests(GetContestsContext context) {
    if (currentContest == nullptr)
        return kj::READY_NOW;

    auto itr = index.iterator_to(*currentContest);
    if (itr == index.end()) {
        currentContest = nullptr;
        return kj::READY_NOW;
    }

    kj::Vector<const Contest*> contestsToReturn;
    while (++itr != index.end() && contestsToReturn.size() < context.getParams().getCount()) {
        if (itr->endTime < fc::time_point(db.head_block_time()))
            continue;
        contestsToReturn.add(&*itr);
    }

    auto results = context.initResults().initNextContests(contestsToReturn.size());
    for (auto i = 0u; i < results.size(); ++i) {
        currentContest = contestsToReturn[i];
        populateContest(results[i]);
    }

    if (itr == index.end())
        currentContest = nullptr;
    else
        currentContest = &*itr;
    return kj::READY_NOW;
}

::kj::Promise<void> swv::FeedGenerator::logEngagement(LogEngagementContext context) {
    // TODO
}

} // namespace swv

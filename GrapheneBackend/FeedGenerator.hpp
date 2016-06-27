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
#ifndef FEEDGENERATOR_HPP
#define FEEDGENERATOR_HPP

#include "Contest.hpp"
#include "Utilities.hpp"

#include <contestgenerator.capnp.h>

#include <graphene/chain/database.hpp>

#include <kj/vector.h>

namespace swv {

template<typename Index>
class FeedGenerator : public ContestGenerator::Server {
public:
    /**
     * @brief The FilterResult enum contains the various result codes a Filter on the FeedGenerator may return
     */
    enum FilterResult {
        Accept, ///< The filter accepts this contest
        Reject, ///< The filter rejects this contest
        Break   ///< The filter rejects this and all future contests
    };
    /**
     * @brief A Filter is a callable which determines whether a contest will be shown in the feed or not.
     *
     * A filter may either Accept or Reject a contest. If all applicable filters Accept a given contest, that contest
     * will be shown in the feed. If any filter Rejects a given contest, that contest will not be shown in the feed.
     *
     * As an optimization, filters may also break on a contest, which indicates that this filter rejects the current
     * contest and will reject all future contests regardless of their content. When any filter breaks on a contest, it
     * is safe for the FeedGenerator to indicate the end of the feed, since it is guaranteed that no more contests can
     * be matched.
     *
     * It is guaranteed that no particular filter will be run twice on the same contest.
     */
    using Filter = std::function<FilterResult(const Contest&, const gch::database&)>;

    FeedGenerator(const Contest* firstContest, const gch::database& db, std::vector<Filter> filters = {});
    virtual ~FeedGenerator();

protected:
    // ContestGenerator::Server interface
    virtual ::kj::Promise<void> getContest(GetContestContext context) override;
    virtual ::kj::Promise<void> getContests(GetContestsContext context) override;
    virtual ::kj::Promise<void> logEngagement(LogEngagementContext context) override;

private:
    const Contest* currentContest = nullptr;
    const gch::database& db;
    std::vector<Filter> filters;
    const typename ContestObjectMultiIndex::index<Index>::type& index;
    // Cache the results of filters, so we make sure we don't call a filter on the same contest twice
    mutable std::map<gch::operation_history_id_type, FilterResult> filterCache;

    void populateContest(ContestGenerator::ListedContest::Builder nextContest);
    FilterResult filter(const Contest& c) const {
        auto itr = filterCache.find(c.contestId);
        if (itr != filterCache.end())
            return itr->second;

        auto result = Accept;
        for (auto& filter : filters) {
            switch (filter(c, db)) {
            case Break:
                filterCache[c.contestId] = Break;
                return Break;
            case Reject:
                result = Reject;
            }
        }
        filterCache[c.contestId] = result;
        return result;
    }
};

template<typename Index>
FeedGenerator<Index>::FeedGenerator(const Contest* firstContest, const graphene::chain::database& db,
                                    std::vector<Filter> filters)
    : currentContest(firstContest),
      db(db),
      index(db.get_index_type<ContestIndex>().indices().get<Index>()),
      filters(kj::mv(filters)) {}

template<typename Index>
FeedGenerator<Index>::~FeedGenerator(){}

template<typename Index>
::kj::Promise<void> FeedGenerator<Index>::getContest(ContestGenerator::Server::GetContestContext context) {
    if (currentContest == nullptr)
        return kj::READY_NOW;

    auto itr = index.iterator_to(*currentContest);
    if (itr == index.end()) {
        currentContest = nullptr;
        return kj::READY_NOW;
    }
    // Skip past all ineligible contests
    while (!itr->isActive(db) || filter(*itr) != Accept) {
        // If a filter broke, or we've checked all contests, kill the generator
        if (filter(*itr) == Break || ++itr == index.end()) {
            currentContest = nullptr;
            return kj::READY_NOW;
        }
    }
    currentContest = &*itr;
    populateContest(context.initResults().initNextContest());

    return kj::READY_NOW;
}

template<typename Index>
::kj::Promise<void> FeedGenerator<Index>::getContests(ContestGenerator::Server::GetContestsContext context) {
    if (currentContest == nullptr)
        return kj::READY_NOW;

    auto itr = index.iterator_to(*currentContest);
    if (itr == index.end()) {
        currentContest = nullptr;
        return kj::READY_NOW;
    }

    kj::Vector<const Contest*> contestsToReturn;
    while (++itr != index.end() && contestsToReturn.size() < context.getParams().getCount()) {
        // If the contest is inactive, skip it
        if (!itr->isActive(db))
            continue;
        // If the contest is not accepted, skip it, but if it breaks a filter, kill the generator too
        if (filter(*itr) != Accept) {
            if (filter(*itr) == Break) {
                currentContest = nullptr;
                return kj::READY_NOW;
            }
            continue;
        }
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

template<typename Index>
::kj::Promise<void> FeedGenerator<Index>::logEngagement(ContestGenerator::Server::LogEngagementContext context) {
    // TODO
}

template<typename Index>
void FeedGenerator<Index>::populateContest(ContestGenerator::ListedContest::Builder nextContest) {
    nextContest.getContestId().setOperationId(currentContest->contestId.instance);
    nextContest.setTracksLiveResults(false);

    // Shorter type names
    using contestantResult = typename decltype(currentContest->contestantResults)::value_type;
    using writeInResult = typename decltype(currentContest->writeInResults)::value_type;
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

} // namespace swv

#endif // FEEDGENERATOR_HPP

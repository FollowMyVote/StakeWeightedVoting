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

#include "Objects/Contest.hpp"
#include "LiveContestResultsServer.hpp"
#include "Utilities.hpp"
#include "VoteDatabase.hpp"

#include <contestgenerator.capnp.h>
#include <generator.capnp.h>

#include <kj/vector.h>
#include <kj/debug.h>

using ContestGenerator = Generator<::ContestInfo>;

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

    FeedGenerator(const Contest* firstContest, VoteDatabase& vdb, std::vector<Filter> filters = {});
    virtual ~FeedGenerator();

protected:
    // ContestGenerator::Server interface
    virtual ::kj::Promise<void> getValues(GetValuesContext context) override;

private:
    const Contest* currentContest = nullptr;
    VoteDatabase& vdb;
    std::vector<Filter> filters;
    const typename ContestObjectMultiIndex::index<Index>::type& index;
    // Cache the results of filters, so we make sure we don't call a filter on the same contest twice
    mutable std::map<gch::operation_history_id_type, FilterResult> filterCache;

    void populateContest(::ContestInfo::Builder nextContest);
    FilterResult filter(const Contest& c) const {
        auto itr = filterCache.find(c.contestId);
        if (itr != filterCache.end())
            return itr->second;

        auto result = Accept;
        for (auto& filter : filters) {
            switch (filter(c, vdb.db())) {
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
FeedGenerator<Index>::FeedGenerator(const Contest* firstContest, VoteDatabase& vdb,
                                    std::vector<Filter> filters)
    : currentContest(firstContest),
      vdb(vdb),
      index(vdb.db().get_index_type<ContestIndex>().indices().get<Index>()),
      filters(kj::mv(filters)) {}

template<typename Index>
FeedGenerator<Index>::~FeedGenerator(){}

template<typename Index>
::kj::Promise<void> FeedGenerator<Index>::getValues(GetValuesContext context) {
    if (currentContest == nullptr)
        return kj::READY_NOW;

    auto itr = index.iterator_to(*currentContest);
    if (itr == index.end()) {
        currentContest = nullptr;
        return kj::READY_NOW;
    }

    kj::Vector<const Contest*> contestsToReturn;
    while (itr != index.end() && contestsToReturn.size() < context.getParams().getCount()) {
        auto& contest = *itr++;
        // If the contest is inactive, skip it
        if (!contest.isActive(vdb.db()))
            continue;
        // If the contest is not accepted, skip it, but if it breaks a filter, kill the generator too
        if (filter(contest) != Accept) {
            if (filter(contest) == Break) {
                currentContest = nullptr;
                return kj::READY_NOW;
            }
            continue;
        }
        contestsToReturn.add(&contest);
    }

    auto results = context.initResults().initValues(contestsToReturn.size());
    for (auto i = 0u; i < results.size(); ++i) {
        currentContest = contestsToReturn[i];
        populateContest(results[i].initValue());
    }

    if (itr == index.end())
        currentContest = nullptr;
    else
        currentContest = &*itr;
    return kj::READY_NOW;
}

template<typename Index>
void FeedGenerator<Index>::populateContest(::ContestInfo::Builder nextContest) {
    nextContest.getContestId().setOperationId(currentContest->contestId.instance);
    auto resultsServer = kj::heap<LiveContestResultsServer>(vdb, currentContest->contestId);
    nextContest.setVotingStake(resultsServer->totalVotingStake());
    nextContest.setContestResults(kj::mv(resultsServer));
    // TODO: set engagement notification API
}

} // namespace swv

#endif // FEEDGENERATOR_HPP

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
#include "BackendServer.hpp"
#include "VoteDatabase.hpp"
#include "FeedGenerator.hpp"
#include "ContestResultsServer.hpp"
#include "ContestCreatorServer.hpp"
#include "Utilities.hpp"
#include "Contest.hpp"

#include <fc/io/json.hpp>

namespace swv {

void populateCoinVolumeHistory(Backend::CoinDetails::VolumeHistory::Builder builder,
                               int32_t historyLength, const CoinVolumeHistory& historyRecord) {
    KJ_REQUIRE(historyLength <= 1000000,
               "OK, let's be reasonable here. You don't need a million hours of volume history.");
    if (historyLength <= 0) {
        builder.setNoHistory();
        return;
    }
    auto history = builder.initHistory();
    auto lastUpdate = historyRecord.hourOfLastUpdate.sec_since_epoch();
    history.setHistoryEndTimestamp(lastUpdate * 1000);
    auto histogram = history.initHistogram(historyLength);
    for (int i = 0; i < historyLength; ++i) {
        auto itr = historyRecord.volumeHistogram.find(fc::time_point_sec(lastUpdate - 3600*i));
        if (itr != historyRecord.volumeHistogram.end())
            histogram.set(historyLength - i - 1, itr->second.value);
        else
            histogram.set(historyLength - i - 1, 0);
    }
}

BackendServer::BackendServer(VoteDatabase& db)
    : vdb(db) {}
BackendServer::~BackendServer() {}

::kj::Promise<void> BackendServer::getContestFeed(Backend::Server::GetContestFeedContext context) {
    auto& contestIndex = vdb.contestIndex().indices();
    auto& startTimeIndex = contestIndex.get<ByStartTime>();
    auto itr = startTimeIndex.lower_bound(vdb.db().head_block_time());
    context.initResults().setGenerator(kj::heap<FeedGenerator<ByStartTime>>(itr == startTimeIndex.end()? nullptr
                                                                                                       : &*itr,
                                                                            vdb.db()));
    return kj::READY_NOW;
}

/// @brief Helper template for FilteredGenerator below
///
/// This template helps the filters decide whether to Reject or Break when they stop matching. If the index we're
/// iterating for the search (SearchIndex) is sorted on the Filter's field (FilterIndex), we can Break. Otherwise, we
/// reject.
///
/// For example, if we're iterating contests by creator to search, then the ByCreator filter can break when it sees a
/// contest with the wrong creator. Since in that case, the contests are sorted by creator, we know that when we see
/// the first contest with a different creator, no future contests will have the matching creator again.
///
/// But if we were iterating contests by coin to search, then ByCreator can make no such assumption, and therefore may
/// only Reject, not Break. In this case, though, ByCoin would be able to Break instead of Reject.
///
/// In general: if SearchIndex == FilterIndex, we can Break; otherwise, we must Reject. Accept is always Accept. The
/// exception is if SearchIndex is ById, then there is no optimization available and we always Reject, not Break.
template<typename SearchIndex, typename FilterIndex>
struct ResultSelector {
    const static auto accept = FeedGenerator<SearchIndex>::FilterResult::Accept;
    const static auto reject = FeedGenerator<SearchIndex>::FilterResult::Reject;
};
template<typename SearchIndex>
struct ResultSelector<SearchIndex, SearchIndex> {
    const static auto accept = FeedGenerator<SearchIndex>::FilterResult::Accept;
    const static auto reject = FeedGenerator<SearchIndex>::FilterResult::Break;
};
template<typename FilterIndex>
struct ResultSelector<ById, FilterIndex> {
    const static auto accept = FeedGenerator<ById>::FilterResult::Accept;
    const static auto reject = FeedGenerator<ById>::FilterResult::Reject;
};

/// @brief Helper template for FilteredGenerator below
///
/// This template helps find the first contest to examine when iterating the index. How we find the first contest
/// depends on which index we're iterating, and the filter that causes us to iterate that index. Given this information
/// we can lookup the first matching contest in the index and return that as the firstContest.
///
/// Most calls to findFirstContest won't have the appropriate conditions (because the SearchIndex doesn't match the
/// FilterIndex), and there's no guarantee that these will be called in any convenient order, so if we get a call with
/// incorrect conditions (which can't help find the first contest) after a call with correct conditions (meaning we
/// found the first contest during that call), we need to preserve the result from the older call. For this reason, an
/// explicit null value is provided, which may actually be null, or it may be the result from a prior, successful call
/// of findFirstContest. If we're being called with incorrect conditions to find the first contest, we just return the
/// provided null value, which either carries forward the null, or carries forward the correct first contest.
template<typename SearchIndex, typename FilterIndex, typename IdType>
inline const Contest* findFirstContest(IdType, const gch::database&, const Contest* nullValue) {
    return nullValue;
}
template<>
inline const Contest* findFirstContest<ByCreator, ByCreator>(gch::account_id_type id, const gch::database& db,
                                           const Contest* nullValue) {
    auto& index = db.get_index_type<ContestIndex>().indices().get<ByCreator>();
    auto itr = index.lower_bound(id);
    return itr == index.end()? nullValue : &*itr;
}
template<>
inline const Contest* findFirstContest<ByCoin, ByCoin>(gch::asset_id_type id, const gch::database& db,
                                                const Contest* nullValue) {
    auto& index = db.get_index_type<ContestIndex>().indices().get<ByCoin>();
    auto itr = index.lower_bound(id);
    return itr == index.end()? nullValue : &*itr;
}
template<>
inline const Contest* findFirstContest<ById, ById>(decltype (nullptr), const gch::database& db,
                                                   const Contest* nullValue) {
    auto& index = db.get_index_type<ContestIndex>().indices();
    return index.empty()? nullValue : &*index.begin();
}

template<typename SearchIndex>
ContestGenerator::Client FilteredGenerator(capnp::List<Backend::Filter>::Reader filters, const gch::database& db) {
    std::vector<typename FeedGenerator<SearchIndex>::Filter> filterFunctions;
    using Filter = Backend::Filter::Type;
    using Results = typename FeedGenerator<SearchIndex>::FilterResult;
    const Contest* firstContest = nullptr;

    // For each requested filter, create a FeedGenerator filter for it
    // In each case, be aware that by the time the filter runs, the filters Reader will be gone! Copy any data a filter
    // needs into its state now. Do not attempt to use the Reader from within a filter.
    for (auto filter : filters) {
        if (filter.getType() == Filter::SEARCH_TERMS) {
            KJ_REQUIRE(filter.getArguments().size() > 0, "Search terms filter must have at least one term");
            firstContest = findFirstContest<SearchIndex, ById>(nullptr, db, firstContest);
            std::vector<std::string> terms;
            for (auto term : filter.getArguments())
                terms.emplace_back(term);
            filterFunctions.emplace_back([terms = kj::mv(terms)] (const Contest& contest, const gch::database&) {
                // If contest's matchesKeyword helper matches for any of the terms, accept the contest
                for (const auto& term : terms)
                    return contest.matchesKeyword(term)? Results::Accept : Results::Reject;
            });
        } else if (filter.getType() == Filter::CONTEST_CREATOR) {
            KJ_REQUIRE(filter.getArguments().size() == 1, "Unexpected number of arguments for creator filter");
            try {
                using Selector = ResultSelector<SearchIndex, ByCreator>;
                auto creator = fc::json::from_string(filter.getArguments()[0]).as<gch::account_id_type>();
                firstContest = findFirstContest<SearchIndex, ByCreator>(creator, db, firstContest);
                filterFunctions.emplace_back([creator = kj::mv(creator)] (const Contest& contest,
                                                                          const gch::database&) {
                    // If contest's creator is the creator we're searching for, accept
                    return contest.creator == creator? Selector::accept : Selector::reject;
                });
            } catch (fc::exception& e) {
                KJ_FAIL_REQUIRE("Failure parsing creator argument", filter.getArguments()[0], e.to_detail_string());
            }
        } else if (filter.getType() == Filter::CONTEST_COIN) {
            KJ_REQUIRE(filter.getArguments().size() == 1, "Unexpected number of arguments for coin filter");
            try {
                using Selector = ResultSelector<SearchIndex, ByCoin>;
                auto coin = gch::asset_id_type(std::stoull((std::string)filter.getArguments()[0]));
                firstContest = findFirstContest<SearchIndex, ByCoin>(coin, db, firstContest);
                filterFunctions.emplace_back([coin] (const Contest& contest, const gch::database&) {
                    // If contest's coin matches the coin we're searching for, accept
                    return contest.coin == coin? Selector::accept : Selector::reject;
                });
            } catch (std::invalid_argument&) {
                KJ_FAIL_REQUIRE("Failure parsing coin for coin filter", filter.getArguments()[0]);
            }
        } else if (filter.getType() == Filter::CONTEST_VOTER) {
            KJ_REQUIRE(filter.getArguments().size() == 1, "Unexpected number of arguments for voter filter");
            try {
                auto voter = fc::json::from_string(filter.getArguments()[0]).as<gch::account_id_type>();
                filterFunctions.emplace_back([voter] (const Contest& contest,
                                             const gch::database& db) {
                    // First, look up the account_balance_object for this voter/coin
                    auto& balanceIndex = db.get_index_type<gch::account_balance_index>().indices()
                                         .get<gch::by_account_asset>();
                    auto balanceItr = balanceIndex.find(boost::make_tuple(voter, contest.coin));
                    // If no balance exists, reject since the voter can't have votes in a coin without a balance in it
                    if (balanceItr == balanceIndex.end())
                        return Results::Reject;
                    // Now try to find a Decision on this contest from the voter we just ID'd. Accept if we find one.
                    auto& index = db.get_index_type<DecisionIndex>().indices().get<ByVoter>();
                    auto itr = index.find(boost::make_tuple(balanceItr->id, contest.contestId));
                    return itr == index.end()? Results::Reject : Results::Accept;
                });
            } catch (fc::exception& e) {
                KJ_FAIL_REQUIRE("Failure parsing voter for voter filter",
                                filter.getArguments()[0], e.to_detail_string());
            }
        }
    }

    return kj::heap<FeedGenerator<SearchIndex>>(firstContest, db, kj::mv(filterFunctions));
}

::kj::Promise<void> BackendServer::searchContests(Backend::Server::SearchContestsContext context) {
    auto filters = context.getParams().getFilters();

    // There are multiple search strategies available to us, depending on which filters are in play. Optimally, we rule
    // out as many contests as possible based on a particular filter and iterate only contests which match that filter,
    // matching against the other filters as we go.

    // If any of the filters searches by coin, we can just iterate contests in that coin, applying the other filters as
    // we go, and stop when we've finished all contests in that coin. We can do this with filters by coin or by
    // creator, so if any of those are available, use that strategy.
    for (auto filter : filters) {
        if (filter.getType() == Backend::Filter::Type::CONTEST_COIN) {
            context.initResults().setGenerator(FilteredGenerator<ByCoin>(filters, vdb.db()));
            return kj::READY_NOW;
        } else if (filter.getType() == Backend::Filter::Type::CONTEST_CREATOR) {
            context.initResults().setGenerator(FilteredGenerator<ByCreator>(filters, vdb.db()));
            return kj::READY_NOW;
        }
    }

    // Another optimizing strategy (not yet implemented) is available when we're searching by voter. In this case, we
    // would iterate the Decision index ByVoter instead of the Contest index, get the set of unique contests the voter
    // has voted on, and apply the remaining filters, then return that set via a ContestGenerator. This is a completely
    // different kind of search strategy to the others though, and at this point I regard it as premature optimization.
    // It's either a lot of work to make a whole new generator type and write all the new filtering code; or it's a lot
    // of work to generalize the existing search infrastructure to the extent that the two strategies can share
    // implementation details. Either way, it's a whole lot of work for an optimization.

    // This is the catch-all case: no optimizing strategy is available, so we just iterate contests by ID and inspect
    // them all.
    context.initResults().setGenerator(FilteredGenerator<ById>(filters, vdb.db()));
    return kj::READY_NOW;
}

::kj::Promise<void> BackendServer::getContestResults(Backend::Server::GetContestResultsContext context) {
    auto contestId = gch::operation_history_id_type(context.getParams().getContestId().getOperationId());
    context.initResults().setResults(kj::heap<ContestResultsServer>(vdb, contestId));
    return kj::READY_NOW;
}

::kj::Promise<void> BackendServer::createContest(Backend::Server::CreateContestContext context) {
    context.initResults().setCreator(kj::heap<ContestCreatorServer>(vdb));
    return kj::READY_NOW;
}

::kj::Promise<void> BackendServer::getCoinDetails(Backend::Server::GetCoinDetailsContext context) {
    auto details = context.initResults().initDetails();
    auto& contestsByCoin = vdb.contestIndex().indices().get<ByCoin>();
    auto coinId = gch::asset_id_type(context.getParams().getCoinId());
    auto range = contestsByCoin.equal_range(coinId);
    details.setActiveContestCount(std::count_if(range.first, range.second, [this](const Contest& c) {
        return c.isActive(vdb.db());
    }));
    details.setTotalContestCount(std::distance(range.first, range.second));

    // TODO: Icon URL
    auto& historyByCoin = vdb.coinVolumeHistoryIndex().indices().get<ByCoin>();
    auto itr = historyByCoin.find(coinId);
    if (itr == historyByCoin.end())
        details.initVolumeHistory().setNoHistory();
    else
        populateCoinVolumeHistory(details.initVolumeHistory(), context.getParams().getVolumeHistoryLength(), *itr);

    return kj::READY_NOW;
}

} // namespace swv

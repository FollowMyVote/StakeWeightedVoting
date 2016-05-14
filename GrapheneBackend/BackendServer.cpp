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
#include "Utilities.hpp"

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
    context.initResults().setGenerator(kj::heap<FeedGenerator>(itr == startTimeIndex.end()? nullptr : &*itr,
                                                               vdb.db()));
    return kj::READY_NOW;
}

::kj::Promise<void> BackendServer::searchContests(Backend::Server::SearchContestsContext context) {
    return KJ_EXCEPTION(UNIMPLEMENTED, "NYI");
}

::kj::Promise<void> BackendServer::getContestResults(Backend::Server::GetContestResultsContext context) {
    auto contestId = unpack<gch::operation_history_id_type>(context.getParams().getContestId());
    context.initResults().setResults(kj::heap<ContestResultsServer>(vdb, contestId));
    return kj::READY_NOW;
}

::kj::Promise<void> BackendServer::createContest(Backend::Server::CreateContestContext context) {
    return KJ_EXCEPTION(UNIMPLEMENTED, "NYI");
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

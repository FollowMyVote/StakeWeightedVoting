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
#include "CoinVolumeHistory.hpp"
#include "Decision.hpp"

#include <graphene/chain/database.hpp>

namespace swv {

void CoinVolumeHistory::recordDecision(const Decision& decision, const gch::database& db) {
    fc::time_point_sec hourTimestamp(db.head_block_time().sec_since_epoch() -
                                     (db.head_block_time().sec_since_epoch() % 3600));

    if (hourTimestamp == hourOfLastUpdate) {
        // Volume has already been recorded during this hour
        if (!activeVotersThisHour.insert(decision.voter).second)
            // Insertion failed, meaning this balance has already been active this hour; don't double-count it
            return;
    } else {
        // This is the first update of the hour! Tally up the votes in the last hour votes were recorded in, and reset
        // the tracking
        for (const auto& voter : activeVotersThisHour)
            volumeHistogram[hourOfLastUpdate] += voter(db).balance;

        hourOfLastUpdate = hourTimestamp;
        activeVotersThisHour.clear();
    }
}

} // namespace swv

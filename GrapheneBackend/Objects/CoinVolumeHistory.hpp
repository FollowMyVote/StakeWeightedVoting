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
#ifndef COINVOLUMEHISTORY_HPP
#define COINVOLUMEHISTORY_HPP

#include "Objects.hpp"

namespace swv {

class CoinVolumeHistory : public gdb::abstract_object<CoinVolumeHistory> {
public:
    static const uint8_t space_id = CoinVolumeHistoryObjectId::space_id;
    static const uint8_t type_id = CoinVolumeHistoryObjectId::type_id;

    /// ID of the coin this volume history is tracking
    gch::asset_id_type coinId;
    /// 1-hour buckets of voting volume, indexed by the time at the beginning of the hour, i.e. 20160513T140000
    std::map<fc::time_point_sec, gch::share_type> volumeHistogram;
    /// Time of the beginning of the hour when this object was last updated
    fc::time_point_sec hourOfLastUpdate;
    /// List of balances which have voted this hour (used to prevent double-counting the same stake voting twice in the
    /// same hour)
    std::set<gch::account_balance_id_type> activeVotersThisHour;

    /**
     * @brief Record the decision for voting volume tracking
     * @param decision Decision to record
     * @param db A reference to the blockchain database
     *
     * In order to get the most accurate volume tally, the volume is not measured at the time of voting, but rather the
     * balance which voted is recorded at the time of voting, and all volume for a given hour is tallied up after the
     * hour ends. This way, ghost volume cannot be created by voting several times with the same stake in the same hour
     * or voting with the stake, moving it to another account, and voting again.
     *
     * Implementation note: Ideally the volume would be tallied as soon as the hour ends, but at present I know of no
     * good way to run a routine to update this object when this happens. A close second would be to process any
     * pending tallies at the maintenance interval; however, graphene does not presently allow plugins to register
     * maintenance tasks. I have requested one or both of these feature to be implemented (graphene issue #650). So for
     * the time being, the tally will not occur for a given coin in a previous hour until some vote occurs for the same
     * coin in a later hour. At that point, the pending tally will be processed and stored. This does potentially
     * distort the results, as stake may be moved after the hour ends, but before another vote is processed which
     * triggers the tally.
     */
    void recordDecision(const Decision& decision, const gch::database& db);
};

struct ByCoin;
using CoinVolumeHistoryMultiIndex = bmi::multi_index_container<
    CoinVolumeHistory,
    bmi::indexed_by<
        bmi::ordered_unique<bmi::tag<gch::by_id>, bmi::member<gch::object, gch::object_id_type, &gch::object::id>>,
        bmi::ordered_unique<bmi::tag<ByCoin>,
            bmi::member<CoinVolumeHistory, gch::asset_id_type, &CoinVolumeHistory::coinId>
        >
    >
>;
using CoinVolumeHistoryIndex = gch::generic_index<CoinVolumeHistory, CoinVolumeHistoryMultiIndex>;

} // namespace swv

FC_REFLECT_DERIVED(swv::CoinVolumeHistory, (graphene::db::object),
                   (coinId)(volumeHistogram))

#endif // COINVOLUMEHISTORY_HPP

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
#ifndef CONTEST_HPP
#define CONTEST_HPP

#include "Types.hpp"

#include <graphene/db/object.hpp>
#include <graphene/db/generic_index.hpp>

namespace swv {

class Contest : public gdb::abstract_object<Contest>
{
public:
    virtual ~Contest();

    static const uint8_t space_id = ContestObjectId::space_id;
    static const uint8_t type_id = ContestObjectId::type_id;

    /// We ID contests by the operation that created them. This is more convenient for clients than the object ID since
    /// clients are not necessarily using FMV nodes.
    gch::operation_history_id_type contestId;
    gch::account_id_type creator;
    std::string name;
    std::string description;
    std::map<std::string, std::string> tags;
    std::map<std::string, std::string> contestants;
    gch::asset_id_type coin;
    fc::time_point creationTime;
    fc::time_point startTime;
    fc::time_point endTime;

    /// Dynamic property -- could be extracted into a separate object to improve performance
    std::map<int32_t, int64_t> contestantResults;
    /// Dynamic property -- could be extracted into a separate object to improve performance
    std::map<std::string, int64_t> writeInResults;
};

struct ByCreator;
struct ByCoin;
struct ByStartTime;
using ContestObjectMultiIndex = bmi::multi_index_container<
    Contest,
    bmi::indexed_by<
        bmi::ordered_unique<bmi::tag<ById>, bmi::member<Contest, gch::operation_history_id_type, &Contest::contestId>>,
        bmi::ordered_non_unique<bmi::tag<ByCreator>, bmi::member<Contest, gch::account_id_type, &Contest::creator>>,
        bmi::ordered_non_unique<bmi::tag<ByCoin>, bmi::member<Contest, gch::asset_id_type, &Contest::coin>>,
        bmi::ordered_non_unique<bmi::tag<ByStartTime>, bmi::member<Contest, fc::time_point, &Contest::startTime>>
    >
>;
using ContestIndex = gch::generic_index<Contest, ContestObjectMultiIndex>;

} // namespace swv

FC_REFLECT_DERIVED(swv::Contest, (graphene::db::object),
                   (contestId)(creator)(name)(description)(tags)(contestants)(coin)(creationTime)(startTime)(endTime)
                   (contestantResults)(writeInResults))

#endif // CONTEST_HPP

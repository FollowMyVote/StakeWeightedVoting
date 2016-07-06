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
#ifndef DECISION_HPP
#define DECISION_HPP

#include "Types.hpp"

#include <boost/multi_index/composite_key.hpp>

namespace swv {

class Decision : public gdb::abstract_object<Decision>
{
public:
    const static uint8_t space_id = DecisionObjectId::space_id;
    const static uint8_t type_id = DecisionObjectId::type_id;

    /// Stored as an operation_history_id_type so clients using non-FMV blockchain nodes can find it
    gch::operation_history_id_type decisionId;
    gch::account_balance_id_type voter;
    gch::operation_history_id_type contestId;
    std::map<int32_t, int32_t> opinions;
    std::vector<std::pair<std::string, std::string>> writeIns;
};

namespace bmi = boost::multi_index;
struct ByVoter;
struct ByContest;
using DecisionObjectMultiIndex = bmi::multi_index_container<
    Decision,
    bmi::indexed_by<
        bmi::ordered_unique<bmi::tag<gch::by_id>, bmi::member<gch::object, gch::object_id_type, &gch::object::id>>,
        bmi::ordered_unique<bmi::tag<ById>,
                            bmi::member<Decision, gch::operation_history_id_type, &Decision::decisionId>>,
        bmi::ordered_unique<bmi::tag<ByVoter>,
                            bmi::composite_key<Decision,
                                bmi::member<Decision, gch::account_balance_id_type, &Decision::voter>,
                                bmi::member<Decision, gch::operation_history_id_type, &Decision::contestId>,
                                bmi::member<Decision, gch::operation_history_id_type, &Decision::decisionId>>>,
        bmi::ordered_non_unique<bmi::tag<ByContest>,
                                bmi::member<Decision, gch::operation_history_id_type, &Decision::contestId>>
    >
>;
using DecisionIndex = gch::generic_index<Decision, DecisionObjectMultiIndex>;

} // namespace swv

FC_REFLECT_DERIVED(swv::Decision, (graphene::db::object),
                   (decisionId)(voter)(contestId)(opinions)(writeIns))

#endif // DECISION_HPP

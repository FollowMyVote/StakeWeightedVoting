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
#ifndef CUSTOMEVALUATOR_HPP
#define CUSTOMEVALUATOR_HPP

#include "Types.hpp"

#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/protocol/custom.hpp>

namespace swv {

/**
 * @brief The CustomEvaluator class implements an evaluator for custom_operation ops, specifically for voting
 */
class CustomEvaluator : public gch::evaluator<CustomEvaluator>
{
public:
    using operation_type = gch::custom_operation;

    gch::void_result do_evaluate(const operation_type& op) { return {}; }
    gch::void_result do_apply(const operation_type& op);

    /// Will be set to true by @ref do_evaluate if there is something to do. False otherwise.
    bool shouldApply = false;
};

} // namespace swv

#endif // CUSTOMEVALUATOR_HPP

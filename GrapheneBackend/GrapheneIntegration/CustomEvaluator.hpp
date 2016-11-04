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

#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/protocol/custom.hpp>

#include <kj/common.h>

namespace gch = graphene::chain;

namespace swv {
class VoteDatabase;

/**
 * @brief The CustomEvaluator class implements an evaluator for custom_operation ops, specifically for voting
 *
 * The voting system uses graphene's custom_operation to update the voting application state. This evaluator processes
 * these operations and determines whether they are relevant to the voting system, and if so, processes them to update
 * the database state.
 *
 * The data in a relevant custom_operation is a serialized Datagram. For the operation types and descriptions of their
 * behavior, see datagram.capnp
 *
 * @note The CustomEvaluator needs a reference to the VoteDatabase. Set this reference using the static setVoteDatabase
 * method before constructing any instances of CustomEvaluator
 */
class CustomEvaluator : public gch::evaluator<CustomEvaluator> {
    static kj::Maybe<VoteDatabase&> vdb;
public:
    using operation_type = gch::custom_operation;

    CustomEvaluator();

    static void setVoteDatabase(VoteDatabase& vdb) {
        CustomEvaluator::vdb = vdb;
    }

    gch::void_result do_evaluate(const operation_type&) { return {}; }
    gch::void_result do_apply(const operation_type& op);
};

} // namespace swv

#endif // CUSTOMEVALUATOR_HPP

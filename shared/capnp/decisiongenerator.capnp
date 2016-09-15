# Copyright 2015 Follow My Vote, Inc.
# This file is part of The Follow My Vote Stake-Weighted Voting Application ("SWV").
#
# SWV is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SWV is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SWV.  If not, see <http://www.gnu.org/licenses/>.

@0x912c38576b075e21;

using DecisionId = import "ids.capnp".DecisionId;
using Generator = import "generator.capnp".Generator;

struct DecisionInfo {
# This is the information about a decision that the server returns to the client when listing decisions on a contest

    id @0 :DecisionId;
    # ID of the decision
    counted @1 :Bool;
    # Whether the decision was counted or not. This would be false on a decision that was later replaced by its owner,
    # for example. For every {voter, contest} pair, there may be several decisions, but exactly one should be counted.
}

using DecisionGenerator = Generator(DecisionInfo);
# The generator for fetching decisions on a contest

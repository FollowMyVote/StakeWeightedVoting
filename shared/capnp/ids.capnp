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

@0xc8496befe955c25b;

# This file defines the contents of nontrivial ID fields for various objects in the system

struct BalanceId {
# The content of the id field on a balance
    accountInstance @0 :UInt64;
    # The instance of the ID of the account which owns the balance
    coinInstance @1 :UInt64;
    # The instance of the ID of the asset the balance is in
}

struct ContestId {
    operationId @0 :UInt64;
    # Instance of the operation_history_id_type for the operation that created this contest
}

struct DecisionId {
    operationId @0 :UInt64;
    # Instance of the operation_history_id_type for the operation that created this decision
}

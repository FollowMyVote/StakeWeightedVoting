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

@0xdf8ddd99d4d9f0ae;

struct Balance {
    id @0 :Data;
    # The ID of this balance
    type @1 :UInt64;
    # The ID of the Coin this balance holds
    amount @2 :Int64;
    # The amount of asset in this balance
    creationOrder @3 :Int64;
    # The creation order is a number which is defined to be larger for a newer balance than for an older one,
    # regardless of owner. The origin of this number is blockchain-specific. For example, on Bitcoin, it could be the
    # block height at which the underlying UTXO was created.
}

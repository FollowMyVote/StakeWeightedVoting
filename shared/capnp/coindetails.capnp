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

@0xa6352c27aac94cb9;

struct CoinDetails {
# This type contains the various details and statistics the server tracks on a per-coin basis

    iconUrl @1 :Text;
    # The URL to the icon to display for this coin (may be empty)
    activeContestCount @0 :Int32;
    # The total number of active contests in this coin
    totalContestCount @5 :Int32;
    # The total number of contests, active or inactive, in this coin

    volumeHistory :union {
        noHistory @2 :Void;
        history :group {
            histogram @3 :List(Int64);
            # A histogram of voting volume. Each element records the volume during a one hour period of time.
            historyEndTimestamp @4 :Int64;
            # Timestamp of the last sample in the histogram. Each sample histogram_n is timestamped exactly one hour
            # prior to sample histogram_n+1
        }
    }
}

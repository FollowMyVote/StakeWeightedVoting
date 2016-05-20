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

@0xd073c95fea7a1ec4;

using Map = import "map.capnp".Map;

const contestPublishingAccount :Text = "follow-my-vote";
# The account which publishes contests and accepts payments

struct Contest {
    name @0 :Text;
    description @1 :Text;
    type @2 :Type;
    tallyAlgorithm @3 :TallyAlgorithm;
    tags @4 :Map(Text, Text);
    # Map of key to value
    contestants @5 :Map(Text, Text);
    # Map of name to description
    coin @6 :UInt64;
    startTime @7 :UInt64;
    # Millisecond timestamp of contest beginning
    endTime @8 :UInt64;
    # Millisecond timestamp of contest end

    enum Type {
    # An enumeration of all types of contests known to the system
        oneOfN @0;
        # A contest with N contestants, where the voter selects at most one
    }
    enum TallyAlgorithm {
    # An enumeration of all tally algorithms known to the system
        plurality @0;
        # The contestant with the plurality of votes wins the contest
    }
}

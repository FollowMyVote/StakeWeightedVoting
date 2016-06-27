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

@0xb3e6658f22b0e5d5;

using ContestId = import "ids.capnp".ContestId;

interface ContestGenerator {
    # An API to retrieve an 'infinite stream' of contests a few at a time. This implements a contest feed, where the
    # client can fetch a few contests to start with, and then fetch more as needed. It also supports feedback on the
    # returned contests, so that the client can notify the server of engagement on certain contests allowing the server
    # to select the next contests to be returned to maximize probability of engagement.

    getContest @0 () -> (nextContest :ListedContest);
    # Retrieve one more contest
    getContests @1 (count :Int32) -> (nextContests :List(ListedContest));
    # Retrieve count more contests; may return less than count if no more contests are available

    logEngagement @2 (contest :Data, engagementType :EngagementType);
    # Notify the server of engagement with a particular contest

    enum EngagementType {
        expanded @0;
        # User expanded the contest to see more detail
        voted @1;
        # User voted on the specified contest
    }

    struct ListedContest {
        contestId @0 :ContestId;
        # Chain-specific contest ID
        votingStake @1 :Int64;
        # Total stake voting on the specified contest
        tracksLiveResults @2 :Bool;
        # Whether the backend provides live results for this contest or not
    }

}

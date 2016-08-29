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
using Generator = import "generator.capnp".Generator;
using Notifier = import "notifier.capnp".Notifier;

enum EngagementType {
# The different types of contest engagement the server tracks
    expanded @0;
    # User expanded the contest to see more detail
    voted @1;
    # User voted on the specified contest
    liked @2;
    # User liked or starred or favorited or however you want to say it on the specified contest
}

interface ContestResults {
    results @0 () -> (results :List(TalliedOpinion));
    # Get the current results. Results are returned in no particular order.
    subscribe @1 (notifier :Notifier(List(TalliedOpinion))) -> ();
    # Subscribe to changes to the results. Notifications will be sent until the ContestResults is destroyed.
    # Results are provided in no particular order.

    struct TalliedOpinion {
        contestant :union {
            contestant @0 :Int32;
            writeIn @2 :Text;
        }
        tally @1 :Int64;
    }
}

struct ContestInfo {
    contestId @0 :ContestId;
    # Chain-specific contest ID; the client can use this to retrieve the full contest from the blockchain
    votingStake @1 :Int64;
    # Total stake voting on the specified contest
    contestResults @2 :ContestResults;
    # API to fetch/subscribe to results for the contest
    engagementNotifier @3 :Notifier(Engagement);
    # API to notify the server of engagement on the contest

    struct Engagement {
        type @0 :EngagementType;
    }
}

using ContestGenerator = Generator(ContestInfo);
# The API for a "contest feed." Returns information about contests, including the ID (which the client can use to look
# up the full contest on the blockchain).

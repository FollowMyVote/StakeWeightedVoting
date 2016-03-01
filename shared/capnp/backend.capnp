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

@0x908359cb8da82d06;

using Purchase = import "purchase.capnp".Purchase;
using Notifier = import "purchase.capnp".Notifier;
using ContestGenerator = import "contestgenerator.capnp".ContestGenerator;
using ContestCreator = import "contestcreator.capnp".ContestCreator;

interface Backend {
    # This is the master API to the FMV backend. It provides services related to listing contests, getting contest
    # results, and purchasing certified reports on the contest results.

    getContestFeed @0 () -> (generator :ContestGenerator);
    # Get a generator for current user's contest feed
    searchContests @1 (filters :List(Filter)) -> (generator :ContestGenerator);
    # Search contests and get a generator for the results
    getContestResults @2 (contestId :Data) -> (results :ContestResults);
    # Get the instantaneous live results for the specified contest

    getContestCreator @3 () -> (creator :ContestCreator);
    # Get a ContestCreator API

    getCoinDetails @4 (coinId :UInt64, volumeHistoryLength :Int32 = -1) -> (details :CoinDetails);
    # Get the details for the given coin
    # volumeHistoryLength is the number of hours to get voting volume history for. If this is nonpositive, no history
    # will be returned.

   interface ContestResults {
        results @0 () -> (results :List(TalliedOpinion));
        # Call results() to get the current results
        subscribe @1 (notifier :Notifier(List(TalliedOpinion))) -> ();
        # Subscribe to changes to the results. Notifications will be sent until the ContestResults is destroyed.

        struct TalliedOpinion {
            contestant @0 :Int32;
            tally @1 :Int64;
        }
    }

    struct Filter {
        type @0 :Type;
        arguments @1 :List(Text);

        enum Type {
            searchTerms @0;
            # Search for contests containing the specified search terms. Each search term is an argument
            contestCreator @1;
            # Search for contests created by the specified account. Argument is a blockchain-specific account ID
            contestCoin @2;
            # Search for contests weighted by the specified coin. Argument is base-10 string of coin ID
            contestVoter @3;
            # Search for contests voted on by the current user. No argument.
        }
    }

    struct CoinDetails {
        # This type contains the various details and statistics the server tracks on a per-coin basis

        iconUrl @1 :Text;
        # The URL to the icon to display for this coin (may be empty)
        activeContestCount @0 :Int32;
        # The total number of active contests in this coin

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
}

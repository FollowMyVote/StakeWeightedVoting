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

using ContestId = import "ids.capnp".ContestId;
using Purchase = import "purchase.capnp".Purchase;
using Notifier = import "purchase.capnp".Notifier;
using ContestGenerator = import "contestgenerator.capnp".ContestGenerator;
using ContestCreator = import "contestcreator.capnp".ContestCreator;
using CoinDetails = import "coindetails.capnp".CoinDetails;
using Generator = import "generator.capnp".Generator;

interface Backend {
    # This is the master API to the FMV backend. It provides services related to listing contests, getting contest
    # results, and purchasing certified reports on the contest results.

    getContestFeed @0 () -> (generator :ContestGenerator);
    # Get a generator for current user's contest feed
    searchContests @1 (filters :List(Filter)) -> (generator :ContestGenerator);
    # Search contests and get a generator for the results

    createContest @2 () -> (creator :ContestCreator);
    # Get a ContestCreator API

    getCoinDetails @3 (coinId :UInt64, volumeHistoryLength :Int32 = -1) -> (details :CoinDetails);
    # Get the details for the given coin
    # volumeHistoryLength is the number of hours to get voting volume history for. If this is nonpositive, no history
    # will be returned.

    struct Filter {
        type @0 :Type;
        arguments @1 :List(Text);

        enum Type {
            searchTerms @0;
            # Search for contests containing the specified search terms. Each search term is an argument
            contestCreator @1;
            # Search for contests created by the specified account. Argument is a JSON account_id_type
            contestCoin @2;
            # Search for contests weighted by the specified coin. Argument is base-10 string of coin ID
            contestVoter @3;
            # Search for contests voted on by the specified user. Argument is a JSON account_id_type
        }
    }
}

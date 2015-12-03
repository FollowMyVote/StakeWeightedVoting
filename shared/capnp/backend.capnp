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

interface Backend {
    # This is the master API to the FMV backend. It provides services related to listing contests, getting contest
    # results, and purchasing certified reports on the contest results.

    increment @8 (num :UInt8) -> (result :UInt8);

    getContestGenerator @0 () -> (generator :ContestGenerator);
    # Get a generator for a feed of contests
    getContestResults @1 (contestId :Data) -> (results :ContestResults);
    # Get the instantaneous live results for the specified contest

    purchaseResultReport @2 (contestId :Data, timestamp :UInt32) -> (api :Purchase);
    # Purchase a contest results report for a given time
    downloadResultReport @3 (contestId :Data, timestamp :UInt32) -> (report :Data);
    # Download a particular contest results report
    listAvailableResultReports @4 (contestId :Data) -> (reports :List(UInt32));
    # Get a list of timestamps of available reports for a given contest

    purchaseAuditTrail @5 (contestId :Data, timestamp :UInt32) -> (api: Purchase);
    # Purchase an audit trail for a result report
    downloadAuditTrail @6 (contestId :Data, timestamp :UInt32) -> (report :Data);
    # Download a particular report's audit trail
    listAvailableAuditTrails @7 (contestId :Data) -> (reports :List(UInt32));
    # Get a list of timestamps of available audit trails for a given contest

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
}

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

interface ContestGenerator {
    # An API to retrieve an 'infinite stream' of contests a few at a time. This implements a contest feed, where the
    # client can fetch a few contests to start with, and then fetch more as needed. It also supports feedback on the
    # returned contests, so that the client can notify the server of engagement on certain contests allowing the server
    # to select the next contests to be returned to maximize probability of engagement.

    next @0 () -> (nextContest :ListedContest);
    # Retrieve one more contest
    nextCount @1 (count :Int32) -> (nextContests :List(ListedContest));
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
        contestId @0 :Data;
        # Chain-specific contest ID
        votingStake @1 :Int64;
        # Total stake voting on the specified contest
        tracksLiveResults @2 :Bool;
        # Whether the backend provides live results for this contest or not
    }

}

interface Purchase {
    # A purchase API is created and sent to a client whenever the client wishes to make a purhcase. The client can
    # check the completion of the purchase, get a list of prices, notify the server that payment has been sent and be
    # notified when the payment is processed. Note that the completion notification does not use the parameter; the
    # server will set it to some empty value.

    complete @0 () -> (result :Bool);
    # Check the status of the purchase
    prices @1 (promoCode :Text) -> (prices :List(Price));
    # Get the price of the purchase in all available coins
    subscribe @2 (notifier :Notifier) -> ();
    # Get notified when the purchase is complete
    paymentSent @3 (priceSelection :Price) -> ();
    # Used to notify the server that the payment has been sent

    struct Price {
        coinId @0 :UInt64;
        amount @1 :Int64;
        payAddress @2 :Text;
    }
}

interface Notifier(T) {
    # Notifiers are used to send a notification of some event. The party requesting notification creates the notifier
    # and sends it to the party providing notification. The notifying party then calls notify when the event pending
    # notification occurs. If the event is a single-occurrence event, and it has already occurred when the notifying
    # party receives the Notifier, that party should send the notification immediately; not allow it to go unnotified.

    notify @0 (message :T) -> ();
}

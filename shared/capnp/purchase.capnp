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

@0x926d0fcd95f1e5d5;

interface Purchase {
    # A purchase API is created and sent to a client whenever the client wishes to make a purhcase. The client can
    # check the completion of the purchase, get a list of prices, notify the server that payment has been sent and be
    # notified when the payment is processed. Note that the completion notification does not use the parameter; the
    # server will set it to some empty value.

    complete @0 () -> (result :Bool);
    # Check the status of the purchase
    prices @1 () -> (prices :List(Price));
    # Get the price of the purchase in all available coins
    subscribe @2 (notifier :Notifier(Text)) -> ();
    # Get notified when the purchase is complete. Notification message will either be "true" or "false"
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

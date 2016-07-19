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

@0x9b313a7bbe328386;

using Coin = import "coin.capnp".Coin;
using DecisionId = import "ids.capnp".DecisionId;
using BalanceId = import "ids.capnp".BalanceId;
using ContestId = import "ids.capnp".ContestId;
using Balance = import "balance.capnp".Balance;
using Contest = import "contest.capnp".Contest;
using Datagram = import "datagram.capnp".Datagram;
using Signed = import "signed.capnp".Signed;

interface BlockchainWallet {
# This interface defines the API between the stake-weighted voting app and a wallet for the blockchain

    getCoinById @0 (id :UInt64) -> (coin :Coin);
    # Get a coin by ID
    getCoinBySymbol @1 (symbol :Text) -> (coin :Coin);
    # Get the coin with the specified symbol
    getAllCoins @2 () -> (coins :List(Coin));
    # Get all coins known to the blockchain

    listMyAccounts @3 () -> (accountNames :List(Text));
    # Get a list of accounts owned by this wallet

    getBalance @4 (id :BalanceId) -> (balance :Balance);
    # Get a balance by ID
    getBalancesBelongingTo @5 (owner :Text) -> (balances :List(Balance));
    # Get all balances belonging to the specified owner

    getContestById @6 (id :ContestId) -> (contest :Signed(Contest));
    # Get a contest by ID

    getDatagramByBalance @7 (balanceId :BalanceId, key :Datagram.DatagramKey) -> (datagram :Datagram);
    # Get the datagram belonging to the specified balance and having the specified type and key

    publishDatagram @8 (datagram :Datagram, payingBalance :BalanceId,
                        publishingBalance :BalanceId) -> (transactionId :Text);
    # Publish the specified datagram to the blockchain, paying the fee from payingBalance and publishing on behalf of
    # publishingBalance (payingBalance and publishingBalance must belong to the same account). This call will complete
    # after the publishing transaction has been broadcast, but before it is confirmed. Return value is the hex-encoded
    # ID of the publishing transaction

    transfer @9 (sendingAccount :Text, receivingAccount :Text,
                 amount :Int64, coinId :UInt64, memo :Text) -> (transactionId :Text);
    # Transfer specified amount from sendingAccount to receivingAccount with memo. Memo will not be encrypted. This
    # call will complete after the transfer transaction has been broadcast, but before it is confirmed. Return value is
    # the hex-encoded ID of the transfer transaction.

    getSharedSecret @10 (myAccountNameOrId :Text, otherAccountNameOrId :Text) -> (secret :Data);
    # Get the shared secret between my account's private memo key and other account's public memo key
    # Returns the 512-bit shared secret
}

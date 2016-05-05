/*
 * Copyright 2015 Follow My Vote, Inc.
 * This file is part of The Follow My Vote Stake-Weighted Voting Application ("SWV").
 *
 * SWV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SWV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SWV.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BLOCKCHAINADAPTORINTERFACE_H
#define BLOCKCHAINADAPTORINTERFACE_H

#include <QByteArray>

#include <kj/async.h>
#include <kj/common.h>
#include <kj/debug.h>

#include "capnp/balance.capnp.h"
#include "capnp/coin.capnp.h"
#include "capnp/datagram.capnp.h"
#include "capnp/contest.capnp.h"
#include "capnp/signed.capnp.h"

/**
 * @brief The BlockchainAdaptorInterface class defines an interface which blockchain adaptors must implement
 *
 * This defines the API that connects the Follow My Vote voting application to an underlying blockchain and wallet. To
 * port the application to a new blockchain or wallet, create an implementation of this interface which uses that
 * wallet and chain.
 */
class BlockchainAdaptorInterface
{
public:
    BlockchainAdaptorInterface() {}
    virtual ~BlockchainAdaptorInterface(){}

    /**
     * @brief Get a coin by ID
     * @param id ID of the coin to retrieve
     * @return Coin having the provided ID
     */
    virtual kj::Promise<Coin::Reader> getCoin(quint64 id) const = 0;
    /**
     * @brief Get a coin by symbol
     * @param symbol Symbol of the coin to retrieve
     * @return Coin having the provided symbol
     */
    virtual kj::Promise<Coin::Reader> getCoin(QString symbol) const = 0;
    /**
     * @brief Get a list of all coins
     * @return A list of all coins known to the system (could be large; avoid calling this frequently)
     */
    virtual kj::Promise<kj::Array<Coin::Reader>> listAllCoins() const = 0;

    /**
     * @brief Get a list of accounts controlled by this interface
     * @return Accounts which can be controlled by this interface
     */
    virtual kj::Promise<kj::Array<QString>> getMyAccounts() const = 0;

    /**
     * @brief Get a balance by ID
     * @param id ID of the balance to retrieve
     * @return Promise for the balance having the provided ID. Promise will be broken if balance is not found.
     */
    virtual kj::Promise<Balance::Reader> getBalance(QByteArray id) const = 0;
    /**
     * @brief Get all balances belonging to the specified owner
     * @param owner Unambiguous ID of the owner; exact semantics are chain-specific
     * @return All balances known to belong to the specified owner
     *
     * The exact contents of the owner string depend on the chain being used. For instance, on a Graphene-based
     * blockchain, owner could be an account name or an account object ID. On Bitcoin, the owner would be an address.
     */
    virtual kj::Promise<kj::Array<Balance::Reader>> getBalancesForOwner(QString owner) const = 0;

    /**
     * @brief Get the contest with the specified ID
     * @param contestId ID of the contest to retrieve
     * @return Promise for the contest having the provided ID. Promise will be broken if contest is not found.
     */
    virtual kj::Promise<::Signed<Contest>::Reader> getContest(QByteArray contestId) const = 0;

    /**
     * @brief Create a datagram for publishing
     * @return A builder which can be populated with a datagram to be published
     *
     * This method creates a new datagram to be published using @ref publishDatagram. Do not call this method a second
     * time without first calling @ref publishDatagram.
     */
    virtual Datagram::Builder createDatagram() = 0;
    /**
     * @brief Publish a datagram to the blockchain
     * @param payerBalance Balance which will be used to pay for the publication
     * @param publisherBalance Balance which the datagram will be stored on
     * @return A promise which will resolve when the datagram is successfully broadcast (not yet confirmed), or broken
     * in case of an error broadcasting the transaction
     *
     * This method will publish the datagram last retrieved using @ref getDatagram using the specified balance. Only
     * one datagram should be retrieved and published at a time.
     *
     * This datagram will replace any datagram with the same type/key which is already owned by the specified balance.
     * Datagrams owned by the balance with a distinct type/key will be preserved.
     */
    virtual kj::Promise<void> publishDatagram(QByteArray payerBalance, QByteArray publisherBalance) = 0;
    [[deprecated("Issue #6: Replaced by overload which distinguishes between payer and publisher balances")]]
    virtual kj::Promise<void> publishDatagram(QByteArray payerBalanceId) {
        KJ_LOG(DBG, "Call to deprecated overload of publishDatagram");
        return publishDatagram(payerBalanceId, payerBalanceId);
    }

    /**
     * @brief transfer funds from one account to another
     * @param sender Name of the account to send from (should be one of the names returned by \ref getMyAccounts)
     * @param recipient Name of the account or address to send to
     * @param amount Amount of coin to send
     * @param coinId Type of coin to send
     * @return A promise which resolves when the transaction is successfully broadcast, or breaks if broadcast fails
     */
    virtual kj::Promise<void> transfer(QString sender, QString recipient, qint64 amount, quint64 coinId) = 0;

    /**
     * @brief Get the datagram with the specified type and key belonging to the specified balance
     * @param balanceId ID of the balance owning the requested datagram
     * @param type The type of the requested datagram
     * @param key The key of the requested datagram
     * @return A promise for the requested datagram. The promise will be broken if no datagram is found.
     */
    virtual kj::Promise<Datagram::Reader> getDatagram(QByteArray balanceId,
                                                      Datagram::DatagramType type,
                                                      QString key) const = 0;
};

#endif // BLOCKCHAINADAPTORINTERFACE_H

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

#ifndef CHAINADAPTORWRAPPER_HPP
#define CHAINADAPTORWRAPPER_HPP

#include "capnp/coin.capnp.h"
#include "capnp/datagram.capnp.h"
#include "capnp/blockchainwallet.capnp.h"

#include "DataStructures/Coin.hpp"
#include "DataStructures/Contest.hpp"

#include "vendor/QQmlObjectListModel.h"

#include <QObject>
#include <QtQml>

#include <kj/async.h>

#include <memory>

class BlockchainAdaptorInterface;
class Promise;
class PromiseConverter;

namespace swv {

class BalanceWrapper;

/**
 * @brief The ChainAdaptorWrapper class wraps a BlockchainAdaptorInterface in a more QML-friendly interface
 *
 * Because BlockchainAdaptorInterface is designed to be mostly Qt-independent, it (and its implementors) is not friendly
 * to QML (i.e. it returns types that QML can't inspect or manipulate). This class wraps the interface and provides an
 * interface which exposes QML-friendly wrappers and types to manipulate the Cap'n Proto backend types.
 *
 * The ChainAdaptorWrapper is also responsible for managing decisions on contests, including their persistence.
 */
class BlockchainWalletApi : public QObject
{
    Q_OBJECT

public:
    BlockchainWalletApi(PromiseConverter& promiseConverter, QObject *parent = 0);
    ~BlockchainWalletApi() noexcept;

    /**
     * @brief Set the blockchain client
     * @param chain The BlockchainWallet to use
     */
    void setChain(BlockchainWallet::Client chain);
    /**
     * @brief Get the BlockchainWallet
     * @return Client for the BlockchainWallet, does not confer ownership
     */
    BlockchainWallet::Client chain() {
        return m_chain;
    }

    /**
     * @brief Get a balance by ID
     * @param id ID of the balance to retrieve
     * @return Balance having the provided ID
     *
     * The wrapper maintains ownership of the returned object.
     */
    Q_INVOKABLE Promise* getBalance(QByteArray id);
    /**
     * @brief Get all balances belonging to the specified owner
     * @param owner Unambiguous ID of the owner; exact semantics are chain-specific
     * @return All balances known to belong to the specified owner
     *
     * The exact contents of the owner string depend on the chain being used. For instance, on a Graphene-based
     * blockchain, owner could be an account name or an account object ID. On Bitcoin, the owner would be an address.
     *
     * The wrapper maintains ownership of the returned objects. The returned list will be empty if the adaptor is not
     * set.
     */
    Q_INVOKABLE Promise* getBalancesBelongingTo(QString owner);

    /**
     * @brief Get the contest with the specified ID
     * @param contestId ID of the contest to retrieve
     * @return Contest having the provided ID
     *
     * The returned contest will have its currentDecision set
     */
    Q_INVOKABLE Promise* getContest(QString contestId);

    /**
     * @brief Get the on-chain decision for the specified owner and contest
     * @param owner Unambiguous ID of the owner; exact semantics are chain-specific
     * @param contestId ID of the contest to get a decision for
     * @return Promise for the decision currently on chain for the specified owner and contest. If no decision is found
     * the promise will be broken
     *
     * This will query all balances in the contest's asset for the given owner and return the newest decision found on
     * any of those balances. If any balance queried has a different decision than the newest, or has no decision, the
     * returned decision will be marked as stale.
     *
     * Avoid calling this function frequently for the same decision as it does not implement any cacheing, so it may be
     * slow and will construct a new Decision on each call.
     */
    Q_INVOKABLE Promise* getDecision(QString owner, QString contestId);
    /// @brief Identical to getDecision, but returns a kj::Promise instead of a Promise*. For C++ use.
    kj::Promise<OwningWrapper<DecisionWrapper>*> _getDecision(QString owner, QString contestId);

    /**
     * @brief Transfer amount from sender to recipient
     * @param sender Account name of sender
     * @param recipient Account name/address of recipient
     * @param amount Amount to send
     * @param coinId Type of coin to send
     * @return A promise which resolves when the transaction has been broadcast, or breaks if transfer fails
     */
    Q_INVOKABLE Promise* transfer(QString sender, QString recipient, qint64 amount, quint64 coinId);

signals:
    void error(QString message);

    /// This signal will be emitted when an event occurs which causes a contest which had previously been counted
    /// correctly to not be counted correctly anymore. Possible causes include the decision going stale because it's
    /// balance was destroyed, the contest's owner acquired a new balance in the relevant coin, the decision being
    /// forked out of the blockchain, etc.
    void contestActionRequired(QString contestId);

protected:
    capnp::RemotePromise<BlockchainWallet::GetContestByIdResults> getContestImpl(QString contestId);
    capnp::RemotePromise<BlockchainWallet::GetBalancesBelongingToResults> getBalancesBelongingToImpl(QString owner);

private:
    PromiseConverter& promiseConverter;
    BlockchainWallet::Client m_chain;
};

} // namespace swv

#endif // CHAINADAPTORWRAPPER_HPP

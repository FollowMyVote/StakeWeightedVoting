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

#include "wrappers/Contest.hpp"
#include "wrappers/Datagram.hpp"

#include <QObject>
#include <QtQml>

#include <memory>

class BlockchainAdaptorInterface;
class Promise;
class PromiseConverter;

namespace swv {

class Coin;
class Balance;

/**
 * @brief The ChainAdaptorWrapper class wraps a BlockchainAdaptorInterface in a more QML-friendly interface
 *
 * Because BlockchainAdaptorInterface is designed to be mostly Qt-independent, it (and its implementors) is not friendly
 * to QML (i.e. it returns types that QML can't inspect or manipulate). This class wraps such an adaptor and provides an
 * interface which exposes QML-friendly wrappers and types to manipulate the Cap'n Proto backend types.
 *
 * The ChainAdaptorWrapper is also responsible for managing decisions on contests, including their persistence.
 */
class ChainAdaptorWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasAdaptor READ hasAdaptor NOTIFY hasAdaptorChanged)
public:
    explicit ChainAdaptorWrapper(PromiseConverter& promiseConverter, QObject *parent = 0);
    ~ChainAdaptorWrapper() noexcept;

    /**
     * @brief Set the blockchain adaptor. The wrapper takes ownership of the adaptor.
     * @param adaptor The adaptor to use
     *
     * If any other adaptor was set previously, it will be destroyed in favor of the new one.
     */
    void setAdaptor(kj::Own<BlockchainAdaptorInterface> m_adaptor);
    /**
     * @brief Get a non-owning pointer to the blockchain adaptor
     * @return Pointer to the adaptor, does not confer ownership
     */
    BlockchainAdaptorInterface* adaptor() {
        return m_adaptor.get();
    }
    /**
     * @brief Extract the blockchain adaptor. The caller takes ownership.
     * @return The blockchain adaptor currently in use
     *
     * The wrapper relinquishes ownership of the adaptor; after this call, the wrapper will have no adaptor until
     * setAdaptor is called.
     */
    kj::Own<BlockchainAdaptorInterface> takeAdaptor();
    /**
     * @brief Check if the adaptor is set
     * @return true if adaptor is set; false otherwise.
     */
    bool hasAdaptor() const {
        return m_adaptor.get() != nullptr;
    }

    /**
     * @brief Get a coin by ID
     * @param id ID of the coin to retrieve
     * @return Coin having the provided ID, or nullptr if adaptor is not set or coin not found
     *
     * The wrapper maintains ownership of the returned object.
     */
    Q_INVOKABLE Promise* getCoin(quint64 id);
    /**
     * @brief Get a coin by symbol
     * @param symbol Symbol of the coin to retrieve
     * @return Coin having the provided symbol, or nullptr if adaptor is not set or coin not found
     *
     * The wrapper maintains ownership of the returned object.
     */
    Q_INVOKABLE Promise* getCoin(QString symbol);
    /**
     * @brief Get a list of all coins
     * @return A list of all coins known to the system (could be large; avoid calling this frequently)
     *
     * The wrapper maintains ownership of the returned objects. The returned list will be empty if the adaptor is not
     * set.
     */
    Q_INVOKABLE Promise* listAllCoins();

    /**
     * @brief Get a list of all accounts controlled by this adaptor
     */
    Q_INVOKABLE Promise* getMyAccounts();

    /**
     * @brief Get a balance by ID
     * @param id ID of the balance to retrieve
     * @return Balance having the provided ID, or nullptr if adaptor is not set or balance not found
     *
     * The wrapper maintains ownership of the returned object.
     */
    Q_INVOKABLE Promise* getBalance(QByteArray id);
    /**
     * @brief Get all balances spendable by a specified account
     * @return All balances controlled by the named account
     *
     * The wrapper maintains ownership of the returned objects. The returned list will be empty if the adaptor is not
     * set.
     */
    Q_INVOKABLE Promise* getAccountBalances(QString account);
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
    Q_INVOKABLE Promise* getBalancesForOwner(QString owner);

    /**
     * @brief Get the contest with the specified ID
     * @param contestId ID of the contest to retrieve
     * @return Contest having the provided ID
     *
     * The returned contest will have its currentDecision set
     */
    Q_INVOKABLE Promise* getContest(QString contestId);

    /**
     * @brief Get a new datagram
     * @return A wrapped datagram, suitable for populating and passing to @ref publishDatagram
     */
    Q_INVOKABLE Datagram* getDatagram();
    /**
     * @brief Publish a datagram to the blockchain
     * @param payer Balance which will be used to pay for the publication
     */
    Q_INVOKABLE void publishDatagram(QByteArray payerBalanceId);
    /**
     * @brief Get datagram with specified schema belonging to specified balance
     * @param balanceId ID of the balance to get datagram for
     * @param schema Schema of the datagram to retrieve
     * @return Promise for the requested datagram or nullptr if adaptor is not ready. Promise will be broken if
     * datagram is not found.
     */
    Q_INVOKABLE Promise* getDatagram(QByteArray balanceId, QString schema = QString::null);

signals:
    void hasAdaptorChanged(bool);
    void error(QString message);

private:
    PromiseConverter& promiseConverter;
    kj::Own<BlockchainAdaptorInterface> m_adaptor;
};

} // namespace swv

#endif // CHAINADAPTORWRAPPER_HPP

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

#ifndef VOTINGSYSTEM_HPP
#define VOTINGSYSTEM_HPP

#include "DataStructures/Account.hpp"
#include "DataStructures/Coin.hpp"

#include "vendor/QQmlObjectListModel.h"

#include <QObject>

#include <kj/async.h>

class Promise;
namespace swv {
namespace data {
class Contest;
}
class BlockchainWalletApi;
class BackendApi;
class Decision;

class VotingSystemPrivate;
/**
 * @brief The VotingSystem class is the high-level interface to the voting system.
 *
 * This class is intended to be created and used from QML directly. It manages the chain adaptor and communication with
 * the backend server. It also manages pending decisions and provides services to the QML that require that C++ touch,
 * such as submitting decisions for broadcast.
 *
 * Before the voting system can be used, it must have a connection to the backend server.
 *
 * The voting system operates within the context of an active or current account. The system reads and writes all chain
 * state relative to this account. In particular, when casting decisions, the current account is the one which the
 * decision will be cast on behalf of. The current account is configured via the currentAccout property.
 *
 * The voting system implements the concept of the Current Decision for each contest. The current decision is the
 * decision which should be displayed in the UI as the decision on a given contest. The current decision may be in one
 * of several states (see @ref Decision::State):
 *  - Pending means the current decision is not cast. This is also the state of an empty (uninitialized) decision
 *  - Cast means the decision has been cast and is stored on the chain, and has not been changed since
 *  - Casting means the decision has been cast, but has not yet been confirmed as stored on the chain
 *  - Stale means the decision has been cast, but the owning balance has been destroyed rendering the decision invalid
 */
class VotingSystem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(swv::data::Account* currentAccount READ currentAccount WRITE setCurrentAccount NOTIFY currentAccountChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY error)
    Q_PROPERTY(bool isBackendConnected READ backendConnected NOTIFY backendConnectedChanged)
    Q_PROPERTY(swv::BlockchainWalletApi* chain READ chain CONSTANT)
    Q_PROPERTY(swv::BackendApi* backend READ backend NOTIFY backendConnectedChanged)
    QML_SORTABLE_OBJMODEL_PROPERTY(data::Coin, coins)
    QML_OBJMODEL_PROPERTY(swv::data::Account, myAccounts)

    Q_DECLARE_PRIVATE(VotingSystem)

public:
    explicit VotingSystem(QObject *parent = 0);
    ~VotingSystem() noexcept;

    QString lastError() const;
    bool backendConnected() const;

    BlockchainWalletApi* chain();
    BackendApi* backend();

    swv::data::Account* currentAccount() const;

    /**
     * @brief Connect to the backend at the specified network endpoint
     * @param hostname Host name or IP of the Follow My Vote server
     * @param port Port of the Follow My Vote server
     * @param myAccountName Account to authenticate with (usually the currentAccount)
     * @return A promise which will resolve or break when the connection succeeds or fails
     *
     * The VotingSystem does not retain ownership of the returned promise; it is the caller's responsibility to delete
     * it. The returned promise does not resolve to any value; it has the semantics of a void promise.
     */
    Q_INVOKABLE Promise* connectToBackend(QString hostname, quint16 port, QString myAccountName);
    /**
      * @brief Initialize the voting system by fetching chain state and account information from the backand and wallet
      * @return A promise which will resolve when initialization is complete
      */
    Q_INVOKABLE Promise* initialize();

    /**
     * @brief Cast the current decision for the given contest
     * @param contest The contest to cast a decision for
     *
     * This method will publish the currentDecision on the specified contest to the chain for the current user. If the
     * decision cannot be cast, an error is emitted.
     *
     * See also @ref cancelCurrentDecision
     */
    Q_INVOKABLE Promise* castCurrentDecision(swv::data::Contest* contest);

    Q_INVOKABLE swv::data::Coin* getCoin(quint64 id);
    Q_INVOKABLE swv::data::Coin* getCoin(QString name);

    Q_INVOKABLE swv::data::Account* getAccount(QString name);

signals:
    /**
     * @brief Notify user of an error which we can't do anything else about
     * @param message Message to show the user describing the error
     *
     * Use this signal for errors which we can't easily recover from, to notify the user that the error occurred. When
     * some smarter form of recovery is possible, implement that error handling logic closer to the code that triggered
     * the error.
     */
    void error(QString message);
    void backendConnectedChanged(bool backendConnected);
    void currentAccountChanged(swv::data::Account* currentAccount);

public slots:
    Promise* configureChainAdaptor(bool useTestingBackend = false);

    /**
     * @brief Cancel changes to the decision on the given contest
     * @param contest The contest to cancel changes on
     *
     * This method will set the currentDeicison on the provided contest to the decision currently on chain for that
     * contest for the current user.
     */
    void cancelCurrentDecision(swv::data::Contest* contest);

    void setCurrentAccount(swv::data::Account* currentAccount);

protected slots:
    void setLastError(QString message);
    void disconnected();

private:
    QScopedPointer<VotingSystemPrivate> d_ptr;
};

} // namespace swv

#endif // VOTINGSYSTEM_HPP

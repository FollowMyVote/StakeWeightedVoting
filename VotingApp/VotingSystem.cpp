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

#include <kj/debug.h>
#include <kj/common.h>

#include <QDebug>
#include <QQmlEngine>

#include "VotingSystem.hpp"
#include "wrappers/Coin.hpp"
#include "wrappers/Balance.hpp"
#include "wrappers/Contest.hpp"
#include "wrappers/Decision.hpp"
#include "wrappers/BackendWrapper.hpp"
#include "wrappers/OwningWrapper.hpp"
#include "wrappers/Converters.hpp"
#include "wrappers/ChainAdaptorWrapper.hpp"
#include "Promise.hpp"
#include "PromiseConverter.hpp"
#include "TwoPartyClient.hpp"

#include "capnqt/QSocketWrapper.hpp"

#include <capnp/message.h>
#include <capnp/serialize-packed.h>

#include <StubChainAdaptor.hpp>

namespace swv {

const static QString PENDING_DECISIONS = QStringLiteral("pendingDecisions");
const static QString PENDING_DECISION = QStringLiteral("pendingDecisions/%1");
const static QString OPINIONS = QStringLiteral("pendingDecisions/%1/opinions");
const static QString WRITEINS = QStringLiteral("pendingDecisions/%1/writeins");

class VotingSystemPrivate : private kj::TaskSet::ErrorHandler {
    Q_DISABLE_COPY(VotingSystemPrivate)
    Q_DECLARE_PUBLIC(VotingSystem)

public:
    VotingSystemPrivate(VotingSystem* q_ptr)
        : q_ptr(q_ptr),
          tasks(*this),
          promiseConverter(kj::heap<PromiseConverter>(tasks)),
          adaptor(kj::heap<ChainAdaptorWrapper>(*promiseConverter)),
          socket(kj::heap<QTcpSocket>())
    {
        // Funky syntax is because QAbstractSocket::error is overloaded.
        // See: http://lists.qt-project.org/pipermail/interest/2013-November/009885.html
        QObject::connect(socket,
                         static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error),
                         q_ptr, [this](QAbstractSocket::SocketError e) {socketError(e);});
    }
    virtual ~VotingSystemPrivate() noexcept
    {}

    VotingSystem* q_ptr;
    QString lastError;
    kj::TaskSet tasks;
    kj::Own<PromiseConverter> promiseConverter;
    kj::Own<ChainAdaptorWrapper> adaptor;
    kj::Own<TwoPartyClient> client;
    kj::Own<BackendWrapper> backend;
    kj::Own<QTcpSocket> socket;
    kj::Own<QSocketWrapper> socketWrapper;
    kj::Array<::Coin::Reader> kjCoins;
    swv::data::Account* currentAccount = nullptr;

    void completeConnection(Promise* connectionPromise) {
        Q_Q(VotingSystem);

        socketWrapper = kj::heap<QSocketWrapper>(*socket);
        client = kj::heap<TwoPartyClient>(*socketWrapper);
        backend = kj::heap<BackendWrapper>(client->bootstrap().castAs<Backend>(), *promiseConverter);
        emit q->backendConnectedChanged(true);
        connectionPromise->resolve({});
        QQmlEngine::setObjectOwnership(connectionPromise, QQmlEngine::JavaScriptOwnership);
    }

    void socketError(QAbstractSocket::SocketError errorCode)
    {
        Q_Q(VotingSystem);

        qDebug() << errorCode;
        if (socket)
            qDebug() << socket->errorString();
        if (q->backendConnected())
            q->disconnected();

        if (errorCode == QAbstractSocket::HostNotFoundError) {
            q->setLastError(QObject::tr("Unable to find Follow My Vote server. Is the voting application up to date?"));
            return;
        }
        if (!q->backendConnected()) {
            q->setLastError(QObject::tr("Unable to connect to Follow My Vote server: %1").arg(socket->errorString()));
            return;
        }
        q->setLastError(QObject::tr("Connection to server has encountered an error: %1").arg(socket->errorString()));
    }

private:
    // ErrorHandler interface
    void taskFailed(kj::Exception&& exception) {
        Q_Q(VotingSystem);

        KJ_LOG(ERROR, exception);
        q->setLastError(QObject::tr("Internal error: Promise failed: %1")
                        .arg(QString::fromStdString(exception.getDescription())));
    }
};

VotingSystem::VotingSystem(QObject *parent)
    : QObject(parent),
      d_ptr(new VotingSystemPrivate(this))
{
    Q_D(VotingSystem);

    PREPARE_SORTABLE_OBJMODEL(coins);

    // Persist the current account
    QObject::connect(this, &VotingSystem::currentAccountChanged, [](swv::data::Account* account) {
        if (account && !account->get_name().isEmpty())
            QSettings().setValue("currentAccount", account->get_name());
        else
            QSettings().remove("currentAccount");
    });

    connect(this, &VotingSystem::backendConnectedChanged, this, &VotingSystem::isReadyChanged);
    connect(this, &VotingSystem::adaptorReadyChanged, this, &VotingSystem::isReadyChanged);
    connect(d->adaptor, &ChainAdaptorWrapper::hasAdaptorChanged, this, &VotingSystem::adaptorReadyChanged);
    connect(d->adaptor, &ChainAdaptorWrapper::error, this, [this](QString error) {
        setLastError(error);
    });
    connect(this, &VotingSystem::isReadyChanged, this, [this, d] {
        if (isReady()) {
            emit ready();

            // Fetch coin list, populate property
            d->promiseConverter->adopt(d->adaptor->adaptor()->listAllCoins().then(
                                          [this, d](kj::Array<::Coin::Reader> coins) {
                for (int i = 0; i < m_coins->count(); ++i)
                    m_coins->get(i)->deleteLater();
                m_coins->clear();
                d->kjCoins = kj::mv(coins);

                // For each coin, fetch the statistics and return a promise for the coin and statistics
                return kj::joinPromises(KJ_MAP(coin, d->kjCoins) {
                         auto request = d->backend->backend().getCoinDetailsRequest();
                         request.setCoinId(coin.getId());
                         // Get one week of volume history
                         request.setVolumeHistoryLength(24 * 7);
                         return request.send().then([coin](capnp::Response<Backend::GetCoinDetailsResults> r) {
                             return std::make_tuple(coin, kj::mv(r));
                         });
                     });
            }).then([this, d](kj::Array<std::tuple<Coin::Reader, capnp::Response<Backend::GetCoinDetailsResults>>> r) {
                // Create wrappers for the coins with statistics set
                for (const auto& tuple : r) {
                    auto wrapper = new CoinWrapper(this);
                    wrapper->updateFields(std::get<0>(tuple));
                    wrapper->updateFields(std::get<1>(tuple).getDetails());
                    m_coins->append(wrapper);
                }
            }));

            // Get my accounts, populate property
            using BalanceList = kj::Array<::Balance::Reader>;
            d->promiseConverter->adopt(d->adaptor->adaptor()->getMyAccounts().then(
                                           [this, d](kj::Array<QString> accountNames) {
                // Get balances for each account
                auto accounts = kj::heapArrayBuilder<kj::Promise<std::tuple<QString,
                                                                            BalanceList>>>(accountNames.size());
                for (QString name : accountNames)
                    accounts.add(d->adaptor->adaptor()->getBalancesForOwner(name).then([name](BalanceList bals) {
                                     return std::make_tuple(name, kj::mv(bals));
                                 }));
                return kj::joinPromises(accounts.finish());
            }).then([this, d](kj::Array<std::tuple<QString, BalanceList>> accountsBalances) {
                // Get the persisted current account name, if any
                auto currentAccountName = QSettings().value("currentAccount").toString();

                // Create Account object with AccountBalances populated, add them to myAccounts list
                for (auto& tuple : accountsBalances) {
                    QString name;
                    BalanceList balances;
                    std::tie(name, balances) = kj::mv(tuple);

                    auto account = new data::Account(this);
                    account->update_name(name);

                    // Sum up balances by coin ID
                    std::map<quint64, qint64> balanceSums;
                    for (Balance::Reader balance : balances)
                        balanceSums[balance.getType()] += balance.getAmount();
                    // Store balance sums in Account object
                    for (auto balPair : balanceSums) {
                        data::AccountBalance balance{balPair.first, balPair.second};
                        account->get_balances()->append(QVariant::fromValue(balance));
                    }

                    m_myAccounts->append(account);
                    // If this account is the persisted current account, set that too
                    if (account->get_name() == currentAccountName)
                        setCurrentAccount(account);
                    // If no account is set yet, go ahead and set the first one we find (we should set a current
                    // account at startup if at all possible)
                    else if (d->currentAccount == nullptr)
                        setCurrentAccount(account);
                }
            }));
        }
    });
}

VotingSystem::~VotingSystem() noexcept
{}

QString VotingSystem::lastError() const {
    Q_D(const VotingSystem);
    return d->lastError;
}

bool VotingSystem::isReady() const {
    return backendConnected() && adaptorReady();
}

bool VotingSystem::backendConnected() const {
    Q_D(const VotingSystem);
    return bool(d->backend);
}

bool VotingSystem::adaptorReady() const {
    Q_D(const VotingSystem);
    return d->adaptor->hasAdaptor();
}

ChainAdaptorWrapper* VotingSystem::adaptor() { Q_D(VotingSystem);
    return d->adaptor;
}

BackendWrapper* VotingSystem::backend() { Q_D(VotingSystem);
    return d->backend;
}

data::Account* VotingSystem::currentAccount() const {
    Q_D(const VotingSystem);

    return d->currentAccount;
}

Promise* VotingSystem::connectToBackend(QString hostname, quint16 port) {
    Q_D(VotingSystem);

    Promise* connectPromise = new Promise(this);

    // I want this connection to be destroyed as soon as it fires for the first time... The best/safest way I can think
    // of to do this is with a shared pointer.
    auto connection = std::make_shared<QMetaObject::Connection>();
    *connection = connect(d->socket, &QTcpSocket::connected, this,
                          [d, connectPromise, connection]() mutable {
        QObject::disconnect(*connection);
        connection.reset();
        d->completeConnection(connectPromise);
    });
    d->socket->connectToHost(hostname, port);
    KJ_LOG(DBG, "Attempting new connection", hostname.toStdString(), port);

    return connectPromise;
}

void VotingSystem::configureChainAdaptor(bool useTestingBackend) {
    Q_D(VotingSystem);

    //TODO: make a real implementation of this
    auto adaptor = kj::heap<StubChainAdaptor>(d->adaptor);
    if (useTestingBackend) {
        d->backend = kj::heap<BackendWrapper>(adaptor->getBackendStub(), *d->promiseConverter);
        emit backendConnectedChanged(true);
    }
    d->adaptor->setAdaptor(kj::mv(adaptor));
}

Promise* VotingSystem::castCurrentDecision(swv::ContestWrapper* contest) {
    Q_D(VotingSystem);

    if (!isReady()) {
        setLastError(tr("Unable to cast vote. Please ensure that you are online and that you have connected this app "
                        "to the blockchain."));
        return nullptr;
    }

    if (d->currentAccount == nullptr) {
        // If this ever happens, it's probably a bug, but better to give an error than crash.
        setLastError(tr("Unable to cast vote because current account is not set. "
                        "Please set your account in the settings and try again."));
        return nullptr;
    }

    if (contest == nullptr) {
        setLastError(tr("Oops! A bug is preventing your vote from being cast. "
                        "(Attempted to cast vote on null contest)"));
        return nullptr;
    }

    auto decision = contest->currentDecision();
    if (decision == nullptr) {
        setLastError(tr("Unable to cast vote because no decision was found."));
        return nullptr;
    }

    auto chain = adaptor();
    if (chain == nullptr) {
        setLastError(tr("Oops! A bug is preventing your vote from being cast. (Chain adaptor is not ready)"));
        return nullptr;
    }

    // Get all balances for current account, filter out the ones in a coin other than this contest's coin
    auto future = chain->adaptor()->getBalancesForOwner(d->currentAccount->get_name());
    auto finishPromise = future.then([this, d, decision, chain, contest](kj::Array<::Balance::Reader> balances) {
        auto newEnd = std::remove_if(balances.begin(), balances.end(), [contest](::Balance::Reader b) {
                           return b.getType() != contest->getCoin();
                       });
        balances = kj::heapArray<::Balance::Reader>(balances.begin(), newEnd);

        if (balances.size() == 0) {
            auto coin = getCoin(contest->getCoin());
            if (coin == nullptr) {
                setLastError(tr("Unable to cast vote because the coin for the contest was not found."));
                KJ_FAIL_REQUIRE("Couldn't cast vote because the contest weight coin was not found.");
            }

            setLastError(tr("Unable to cast vote because the current account, %1, has no %2.")
                         .arg(d->currentAccount->get_name()).arg(coin->get_name()));
            KJ_FAIL_REQUIRE("Couldn't cast vote because voting account has no balances in the coin");
        }

        auto serialDecision = decision->serialize();
        auto promises = kj::heapArrayBuilder<kj::Promise<void>>(balances.size());
        for (auto balance : balances) {
            auto dgram = chain->getNewDatagram();
            dgram.initIndex().setType(Datagram::DatagramType::DECISION);
            dgram.getIndex().setKey(balance.getId());
            dgram.setContent(convertBlob(serialDecision));

            promises.add(chain->adaptor()->publishDatagram(convertBlob(balance.getId())));
        }

        return kj::joinPromises(promises.finish());
    });

    return d->promiseConverter->convert(kj::mv(finishPromise));
}

CoinWrapper* VotingSystem::getCoin(quint64 id)
{
    for (CoinWrapper* coin : m_coins->toList())
        if (coin->get_coinId() == id)
            return coin;
    return nullptr;
}

CoinWrapper* VotingSystem::getCoin(QString name)
{
    for (CoinWrapper* coin : m_coins->toList())
        if (coin->get_name() == name)
            return coin;
    return nullptr;
}

data::Account*VotingSystem::getAccount(QString name)
{
    for (auto account : m_myAccounts->toList())
        if (account->get_name() == name)
            return account;
    return nullptr;
}

void VotingSystem::cancelCurrentDecision(ContestWrapper* contest) {
    Q_D(VotingSystem);

    if (!isReady()) {
        setLastError(tr("Unable to cancel vote. Please ensure that you are online and that you have connected this "
                        "app to the blockchain."));
        return;
    }
    if (d->currentAccount == nullptr) {
        // If this ever happens, it's probably a bug. Log it, and just reset the decisions.
        KJ_LOG(ERROR, "Current account was unset while canceling a decision. This probably shouldn't be possible.");
        contest->currentDecision()->setOpinions({});
        return;
    }

    auto promise = d->adaptor->_getDecision(currentAccount()->get_name(), contest->id());
    d->tasks.add(promise.then([contest](swv::DecisionWrapper* decision) {
        contest->setCurrentDecision(decision);
    }, [contest](kj::Exception) {
        contest->currentDecision()->setOpinions({});
    }));
}

void VotingSystem::setCurrentAccount(data::Account* currentAccount) {
    Q_D(VotingSystem);

    if (d->currentAccount == currentAccount)
        return;

    d->currentAccount = currentAccount;
    emit currentAccountChanged(currentAccount);
}

void VotingSystem::setLastError(QString message) {
    Q_D(VotingSystem);

    d->lastError = message;
    emit error(message);
}

void VotingSystem::disconnected()
{
    Q_D(VotingSystem);

    d->backend = nullptr;
    d->backend = nullptr;
    d->client = nullptr;
    d->socketWrapper = nullptr;
    d->socket->close();

    emit backendConnectedChanged(false);
}

} // namespace swv

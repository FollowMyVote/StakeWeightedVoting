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

#include "wrappers/Coin.hpp"
#include "wrappers/Balance.hpp"
#include "wrappers/Contest.hpp"
#include "wrappers/Decision.hpp"
#include "wrappers/Datagram.hpp"
#include "wrappers/BackendWrapper.hpp"
#include "wrappers/OwningWrapper.hpp"
#include "VotingSystem.hpp"
#include "ChainAdaptorWrapper.hpp"
#include "Promise.hpp"
#include "PromiseWrapper.hpp"
#include "TwoPartyClient.hpp"

#include "capnqt/QtEventPort.hpp"
#include "capnqt/QSocketWrapper.hpp"

#include <capnp/message.h>
#include <capnp/serialize-packed.h>

#include <StubChainAdaptor.hpp>

namespace swv {

const static QString PENDING_DECISIONS = QStringLiteral("pendingDecisions");
const static QString PENDING_DECISION = QStringLiteral("pendingDecisions/%1");
const static QString OPINIONS = QStringLiteral("pendingDecisions/%1/opinions");
const static QString WRITEINS = QStringLiteral("pendingDecisions/%1/writeins");

class VotingSystemPrivate {
    Q_DISABLE_COPY(VotingSystemPrivate)
    Q_DECLARE_PUBLIC(VotingSystem)

public:
    VotingSystemPrivate(VotingSystem* q_ptr)
        : q_ptr(q_ptr),
          eventPort(kj::heap<QtEventPort>()),
          loop(*eventPort.get()),
          waitScope(loop),
          promiseConverter(kj::heap<PromiseConverter>()),
          adaptor(kj::heap<ChainAdaptorWrapper>(*promiseConverter)),
          socket(kj::heap<QTcpSocket>())
    {
        eventPort->setLoop(&loop);
        // Funky syntax is because QAbstractSocket::error is overloaded.
        // See: http://lists.qt-project.org/pipermail/interest/2013-November/009885.html
        QObject::connect(socket,
                         static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error),
                         q_ptr, [this](QAbstractSocket::SocketError e) {socketError(e);});
    }
    ~VotingSystemPrivate() noexcept
    {}

    VotingSystem* q_ptr;
    QString lastError;
    kj::Own<QtEventPort> eventPort;
    kj::EventLoop loop;
    kj::WaitScope waitScope;
    kj::Own<PromiseConverter> promiseConverter;
    kj::Own<ChainAdaptorWrapper> adaptor;
    kj::Own<TwoPartyClient> client;
    kj::Own<BackendWrapper> backend;
    kj::Own<QTcpSocket> socket;
    kj::Own<QSocketWrapper> socketWrapper;
    QString currentAccount;

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
};

VotingSystem::VotingSystem(QObject *parent)
    : QObject(parent),
      d_ptr(new VotingSystemPrivate(this))
{
    Q_D(VotingSystem);

    connect(this, &VotingSystem::backendConnectedChanged, this, &VotingSystem::isReadyChanged);
    connect(this, &VotingSystem::adaptorReadyChanged, this, &VotingSystem::isReadyChanged);
    connect(d->adaptor, &ChainAdaptorWrapper::hasAdaptorChanged, this, &VotingSystem::adaptorReadyChanged);
    connect(d->adaptor, &ChainAdaptorWrapper::error, this, [this](QString error) {
        setLastError(error);
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

ChainAdaptorWrapper* VotingSystem::adaptor() {
    Q_D(VotingSystem);
    return d->adaptor;
}

BackendWrapper* VotingSystem::backend() {
    Q_D(VotingSystem);
    return d->backend;
}

QString VotingSystem::currentAccount() const
{
    Q_D(const VotingSystem);

    return d->currentAccount;
}

Promise* VotingSystem::connectToBackend(QString hostname, quint16 port)
{
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

void VotingSystem::configureChainAdaptor()
{
    Q_D(VotingSystem);

    //TODO: make a real implementation of this
    auto adaptor = kj::heap<StubChainAdaptor>(d->adaptor);
    d->adaptor->setAdaptor(kj::mv(adaptor));
}

void VotingSystem::resetCurrentDecision(Contest* contest)
{
    //TODO: implement me
}

void VotingSystem::castCurrentDecision(swv::Contest* contest)
{
    Q_D(VotingSystem);

    if (contest == nullptr) {
        setLastError(tr("Oops! A bug is preventing your vote from being cast. "
                        "(Attempted to cast vote on null contest)"));
        return;
    }

    auto decision = contest->currentDecision();
    if (decision == nullptr) {
        setLastError(tr("Unable to cast vote because no decision was found."));
        return;
    }
    if (decision->state() == Decision::Cast) {
        setLastError(tr("Unable to cast vote because it has not been changed since it was last cast."));
        return;
    }

    auto chain = adaptor();
    if (chain == nullptr) {
        setLastError(tr("Oops! A bug is preventing your vote from being cast. (Chain adaptor is not ready)"));
        return;
    }

    decision->setState(Decision::Casting);

    // Get all balances for current account, filter out the ones in a coin other than this contest's coin
    auto future = chain->adaptor()->getBalancesForOwner(d->currentAccount);
    auto finishPromise = future.then([this, d, decision, chain, contest](kj::Array<::Balance::Reader> balances) {
        auto newEnd = std::remove_if(balances.begin(), balances.end(), [contest](::Balance::Reader b) {
                           return b.getType() != contest->getCoin();
                       });
        balances = kj::heapArray<::Balance::Reader>(balances.begin(), newEnd);

        if (balances.size() == 0) {
            auto coinPromise = chain->getCoin(contest->getCoin());
            auto results = coinPromise->wait();
            if (coinPromise->state() == Promise::State::REJECTED)
                setLastError(tr("Unable to cast vote because the coin for the contest was not found."));

            setLastError(tr("Unable to cast vote because the current account, %1, has no %2.")
                         .arg(d->currentAccount).arg(results.first().value<swv::Coin*>()->name()));
        }

        QString serialDecision = decision->serialize().toHex();
        for (auto balance : balances) {
            auto dgram = chain->getDatagram();
            dgram->setSchema(DECISION_SCHEMA + contest->id());
            dgram->setContent(serialDecision);

            chain->publishDatagram(QByteArray::fromHex(QByteArray(reinterpret_cast<const char*>(balance.getId().begin()))));
        }

        // TODO: detect transaction confirmation and set decision state to Cast
    });

    finishPromise.detach([this] (kj::Exception e) {
        setLastError(QString::fromStdString(e.getDescription()));
    });
}

void VotingSystem::setCurrentAccount(QString currentAccount)
{
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

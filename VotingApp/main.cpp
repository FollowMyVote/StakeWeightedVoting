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

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QDebug>

#include <capnp/rpc-twoparty.h>
#include <capnp/rpc-twoparty.capnp.h>

#include <kj/debug.h>
#include <kj/async-io.h>

#include "wrappers/Coin.hpp"
#include "wrappers/Balance.hpp"
#include "wrappers/Contest.hpp"
#include "wrappers/Decision.hpp"
#include "wrappers/Datagram.hpp"
#include "wrappers/ContestGeneratorWrapper.hpp"
#include "BackendWrapper.hpp"
#include "VotingSystem.hpp"
#include "ChainAdaptorWrapper.hpp"
#include <Promise.hpp>

#include <capnqt/QtEventPort.hpp>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QObject::tr("Stake Weighted Voting"));
    app.setOrganizationName(QStringLiteral("Follow My Vote"));
    app.setOrganizationDomain(QStringLiteral("followmyvote.com"));

    QtEventPort eventPort;
    kj::EventLoop loop(eventPort);
    eventPort.setLoop(&loop);
    kj::WaitScope wsc(loop);

#ifndef NDEBUG
    QQmlDebuggingEnabler debugEnabler;
#endif

    qmlRegisterUncreatableType<swv::Balance>("FollowMyVote.StakeWeightedVoting", 1, 0, "Balance",
                                             QStringLiteral("Balances cannot be created from QML."));
    qmlRegisterUncreatableType<swv::Contest>("FollowMyVote.StakeWeightedVoting", 1, 0, "Contest",
                                             QStringLiteral("Contests cannot be created from QML."));
    qmlRegisterUncreatableType<swv::Coin>("FollowMyVote.StakeWeightedVoting", 1, 0, "Coin",
                                          QStringLiteral("Coins cannot be created from QML."));
    qmlRegisterUncreatableType<swv::Datagram>("FollowMyVote.StakeWeightedVoting", 1, 0, "Datagram",
                                              QStringLiteral("Datagrams cannot be created from QML."));
    qmlRegisterUncreatableType<swv::Decision>("FollowMyVote.StakeWeightedVoting", 1, 0, "Decision",
                                              QStringLiteral("Decisions cannot be created from QML."));
    qmlRegisterUncreatableType<swv::BackendWrapper>("FollowMyVote.StakeWeightedVoting", 1, 0, "Backend",
                                                    QStringLiteral("Backend cannot be created from QML."));
    qmlRegisterUncreatableType<swv::ChainAdaptorWrapper>("FollowMyVote.StakeWeightedVoting", 1, 0, "ChainAdaptor",
                                                         QStringLiteral("Chain Adaptor cannot be created from QML."));
    qmlRegisterUncreatableType<swv::ContestGeneratorWrapper>("FollowMyVote.StakeWeightedVoting", 1, 0,
                                                             "ContestGenerator",
                                                             QStringLiteral("Contest Generator cannot be created from "
                                                                            "QML."));
    qmlRegisterType<swv::VotingSystem>("FollowMyVote.StakeWeightedVoting", 1, 0, "VotingSystem");
    qmlRegisterType<Promise>("FollowMyVote.StakeWeightedVoting", 1, 0, "Promise");

    QQmlApplicationEngine engine;
    Promise::setEngine(&engine);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}

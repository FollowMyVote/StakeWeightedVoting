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

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QDebug>

#include <capnp/rpc-twoparty.h>
#include <capnp/rpc-twoparty.capnp.h>

#include <kj/debug.h>
#include <kj/async-io.h>

#include <DataStructures/Account.hpp>
#include "wrappers/Coin.hpp"
#include "wrappers/Balance.hpp"
#include "wrappers/Contest.hpp"
#include "wrappers/Decision.hpp"
#include "wrappers/ContestGeneratorWrapper.hpp"
#include "wrappers/PurchaseContestRequest.hpp"
#include "wrappers/PurchaseWrapper.hpp"
#include "wrappers/ContestCreator.hpp"
#include "wrappers/BackendWrapper.hpp"
#include "wrappers/ChainAdaptorWrapper.hpp"
#include "VotingSystem.hpp"
#include <Promise.hpp>

#include <capnqt/QtEventPort.hpp>

#include <QtQmlTricksPlugin.h>

#include <VPApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QObject::tr("Stake Weighted Voting"));
    app.setOrganizationName(QStringLiteral("Follow My Vote"));
    app.setOrganizationDomain(QStringLiteral("followmyvote.com"));

    VPApplication vplay;
    vplay.setPreservePlatformFonts(true);

    QtEventPort eventPort;
    kj::EventLoop loop(eventPort);
    eventPort.setLoop(&loop);
    kj::WaitScope wsc(loop);

#ifndef NDEBUG
    QQmlDebuggingEnabler debugEnabler;
#endif

    // Register data structures
    qmlRegisterType<swv::data::Account>("FollowMyVote.StakeWeightedVoting", 1, 0, "Account");

    // Register data/API wrappers
#define REGISTER_WRAPPER(name) \
    qmlRegisterUncreatableType<swv::name ## Wrapper>("FollowMyVote.StakeWeightedVoting", 1, 0, #name, \
                                          #name " is a wrapper. It cannot be created from QML.")
    REGISTER_WRAPPER(Balance);
    REGISTER_WRAPPER(Decision);
    REGISTER_WRAPPER(Contest);
    REGISTER_WRAPPER(Coin);
    REGISTER_WRAPPER(Backend);
    REGISTER_WRAPPER(ChainAdaptor);
    REGISTER_WRAPPER(ContestGenerator);
    REGISTER_WRAPPER(ContestCreator);
    REGISTER_WRAPPER(PurchaseContestRequest);
    REGISTER_WRAPPER(Purchase);
#undef REGISTER_WRAPPER

    // Register enum wrappers
#define REGISTER_ENUM(name) \
    qmlRegisterUncreatableType<swv::name>("FollowMyVote.StakeWeightedVoting", 1, 0, #name, \
                                          #name " is an enum container only; it cannot be created.")
    REGISTER_ENUM(LineItems);
    REGISTER_ENUM(ContestLimits);
    REGISTER_ENUM(ContestType);
    REGISTER_ENUM(TallyAlgorithm);
#undef REGISTER_ENUM

    // Other registrations
    qmlRegisterType<swv::VotingSystem>("FollowMyVote.StakeWeightedVoting", 1, 0, "VotingSystem");
    qmlRegisterType<Promise>("FollowMyVote.StakeWeightedVoting", 1, 0, "Promise");
    qmlRegisterType<QQmlObjectListModelBase>();
    qmlRegisterType<QSortFilterProxyModel>();

    QQmlApplicationEngine engine;
    registerQtQmlTricksUiElements(&engine);
    Promise::setEngine(&engine);
    vplay.initialize(&engine);
    vplay.setMainQmlFileName(QStringLiteral("qrc:/qml/main.qml"));
    engine.load(QUrl(vplay.mainQmlFileName()));

    return app.exec();
}

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
#include <QQuickStyle>
#include <QDebug>

#include <capnp/rpc-twoparty.h>
#include <capnp/rpc-twoparty.capnp.h>

#include <kj/debug.h>
#include <kj/async-io.h>

#include "DataStructures/Account.hpp"
#include "DataStructures/Coin.hpp"
#include "DataStructures/Balance.hpp"
#include "DataStructures/Contest.hpp"
#include "DataStructures/Decision.hpp"
#include "Apis/ContestCreatorApi.hpp"
#include "Apis/BackendApi.hpp"
#include "Apis/BlockchainWalletApi.hpp"
#include "Apis/ContestGeneratorApi.hpp"
#include "Apis/PurchaseContestRequestApi.hpp"
#include "Apis/PurchaseApi.hpp"
#include "Apis/ContestResultsApi.hpp"
#include "Apis/DecisionGeneratorApi.hpp"
#include "VotingSystem.hpp"
#include "qppromise.h"

#include <capnqt/QtEventPort.hpp>

#include <QtQmlTricksPlugin.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
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

    // Register data structures
    qmlRegisterType<swv::data::Account>("FollowMyVote.StakeWeightedVoting", 1, 0, "Account");
    qmlRegisterType<swv::data::Coin>("FollowMyVote.StakeWeightedVoting", 1, 0, "Coin");
    qmlRegisterType<swv::data::Contest>("FollowMyVote.StakeWeightedVoting", 1, 0, "Contest");
    qmlRegisterType<swv::data::Balance>("FollowMyVote.StakeWeightedVoting", 1, 0, "Balance");
    qmlRegisterType<swv::data::Decision>("FollowMyVote.StakeWeightedVoting", 1, 0, "Decision");

    // Register API wrappers
#define REGISTER_API(name) \
    qmlRegisterUncreatableType<swv::name ## Api>("FollowMyVote.StakeWeightedVoting", 1, 0, #name, \
                                          #name " is a QML API. It cannot be created from QML.")
    REGISTER_API(Backend);
    REGISTER_API(BlockchainWallet);
    REGISTER_API(ContestGenerator);
    REGISTER_API(ContestCreator);
    REGISTER_API(PurchaseContestRequest);
    REGISTER_API(Purchase);
    REGISTER_API(ContestResults);
    REGISTER_API(DecisionGenerator);
#undef REGISTER_API

    // Register enum wrappers
#define REGISTER_ENUM(name) \
    qmlRegisterUncreatableType<swv::name>("FollowMyVote.StakeWeightedVoting", 1, 0, #name, \
                                          #name " is an enum container only; it cannot be created.")
    REGISTER_ENUM(LineItems);
    REGISTER_ENUM(ContestLimits);
    REGISTER_ENUM(ContestType);
    REGISTER_ENUM(TallyAlgorithm);
    REGISTER_ENUM(ContestDecisionStatus);
#undef REGISTER_ENUM

    // Other registrations
    qmlRegisterType<swv::VotingSystem>("FollowMyVote.StakeWeightedVoting", 1, 0, "VotingSystem");
    qmlRegisterType<QQmlObjectListModelBase>();
    qmlRegisterType<QSortFilterProxyModel>();

    QQuickStyle::setStyle("Material");

    QQmlApplicationEngine engine;
    QPPromise::setEngine(&engine);
    engine.addImportPath("qrc:/");
    registerQtQmlTricksUiElements(&engine);
    engine.load(QUrl("qrc:/qml/main.qml"));

    return app.exec();
}

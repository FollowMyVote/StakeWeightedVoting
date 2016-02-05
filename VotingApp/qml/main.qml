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

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import Qt.labs.settings 1.0

import VPlayApps 1.0

import FollowMyVote.StakeWeightedVoting 1.0

App {
    id: window
    title: Qt.application.name
    width: 1280
    height: 768
    visible: true
    minimumWidth: dp(400)

    onInitTheme: {
        Theme.platform = "android"
        Theme.colors.backgroundColor = "#e5e5e5"
    }

    function showError(errorMessage) {
        NativeDialog.confirm(qsTr("Error"), qsTr("An error has occurred:\n%1").arg(errorMessage), function(){}, false)
    }

    Action {
        shortcut: "Ctrl+Q"
        onTriggered: Qt.quit()
    }

    Settings {
        id: appSettings
        property alias currentAccount: _votingSystem.currentAccount
    }
    VotingSystem {
       id: _votingSystem

       signal connected

       Component.onCompleted: {
           configureChainAdaptor()
           connectToBackend("127.0.0.1", 2572).then(_votingSystem.connected)
       }
       onError: {
           console.log("Error from Voting System: %1".arg(message))
           showError(message.split(";").slice(-1))
       }
       onIsReadyChanged: {
           console.log("Voting System Ready: " + isReady)
           if (isReady && !currentAccount) {
               currentAccount = Qt.binding(function() {
                   if (adaptor.myAccounts.length)
                       currentAccount = adaptor.myAccounts[0]
               })
           }
       }
       onCurrentAccountChanged: console.log("Current account set to " + currentAccount)
    }

    Navigation {
        id: mainNavigation

        NavigationItem {
            title: qsTr("My Feed")
            icon: IconType.newspapero

            NavigationStack {
                 splitView: false
                 ContestListPage {
                    id: feedPage
                    title: qsTr("My Feed")
                    votingSystem: _votingSystem
                    getContestGeneratorFunction: function() {
                        if (votingSystem.isReady)
                            return votingSystem.backend.getFeedGenerator()
                    }
                    Component.onCompleted: {
                        if (votingSystem.isReady) loadContests()
                        else votingSystem.connected.connect(loadContests)
                    }
                }
            }
        }
        NavigationItem {
            title: qsTr("My Polls")
            icon: IconType.user

            NavigationStack {
                 splitView: false
                 ContestListPage {
                    id: myContestsPage
                    title: qsTr("My Polls")
                    votingSystem: _votingSystem
                    getContestGeneratorFunction: function() {
                        if (votingSystem.isReady)
                            return votingSystem.backend.getContestsByCreator(votingSystem.currentAccount)
                    }
                    listView.headerPositioning: ListView.PullBackHeader
                    listView.header: CreateContestPlaceholder {
                        onClicked: {
                            console.log("Begin contest creation")
                            myContestsPage.navigationStack.push(Qt.resolvedUrl("CreateContestPage.qml"),
                                                                {"contestCreator": _votingSystem.backend.contestCreator,
                                                                 "votingSystem": _votingSystem})
                        }
                    }
                }
            }
        }
        NavigationItem {
            title: "Coin List"
            icon: IconType.money

            NavigationStack {
                splitView: false
                CoinListPage {
                    id: coinListPage
                    votingSystem: _votingSystem
                    Component.onCompleted: if (_votingSystem.isReady) loadCoins()
                }
            }
        }

        NavigationItem {
            title: "Settings"
            icon: IconType.cog

            NavigationStack {
                 splitView: false
                 SettingsPage {
                    votingSystem: _votingSystem
                }
            }
        }
    }
}

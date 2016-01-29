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
        var dialog= InputDialog.confirm(window, qsTr("An error has occurred:\n%1").arg(errorMessage), function(){})
        dialog.negativeAction = false
        dialog.Keys.escapePressed.connect(dialog.close)
        dialog.focus = true
    }

    Action {
        shortcut: "Ctrl+Q"
        onTriggered: Qt.quit()
    }

    VotingSystem {
       id: votingSystem

       signal connected

       Component.onCompleted: {
           configureChainAdaptor()
           connectToBackend("127.0.0.1", 2572).then(votingSystem.connected)
       }
       onError: {
           console.log("Error from Voting System: %1".arg(message))
           showError(qsTr("Internal Error"), message)
       }
       onIsReadyChanged: console.log("Voting System Ready: " + isReady)
       onCurrentAccountChanged: console.log("Current account set to " + currentAccount)
    }

    Navigation {
        NavigationItem {
            title: "Feed"

            NavigationStack {
                ContestListPage {
                    id: feedPage
                    getContestGeneratorFunction: function() {
                        if (votingSystem.isReady)
                            return votingSystem.backend.getFeedGenerator()
                    }
                    Component.onCompleted: if (votingSystem.isReady) loadContests()
                }
            }
        }
        NavigationItem {
            title: "Coin List"

            NavigationStack {
                CoinListPage {
                    id: coinListPage
                    Component.onCompleted: if (votingSystem.isReady) loadCoins()
                }
            }
        }
    }
}

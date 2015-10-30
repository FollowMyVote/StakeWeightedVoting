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

import Material 0.1
import Material.ListItems 0.1

import FollowMyVote.StakeWeightedVoting 1.0

ApplicationWindow {
    title: Qt.application.name
    width: 1280
    height: 768
    visible: true
    initialPage: feedPage

    theme {
        primaryColor: "#2196F3"
        primaryDarkColor: "#1976D2"
        accentColor: "#7C4DFF"
        backgroundColor: "white"
    }

    Action {
        name: qsTr("Quit")
        shortcut: "Ctrl+Q"
        onTriggered: Qt.quit()
    }

    VotingSystem {
       id: votingSystem

       Binding on currentAccount {
           when: navDrawer.selectedAccountIndex >= 0
           value: votingSystem.isReady? navDrawer.accountList[navDrawer.selectedAccountIndex] : null
       }

       Component.onCompleted: {
           configureChainAdaptor()
           connectToBackend("127.0.0.1", 2572).then(feedPage.reloadContests)
       }
       onError: {
           console.log("Error from Voting System: %1".arg(message))
           showError(qsTr("Internal Error"), message)
       }
       onIsReadyChanged: console.log("Voting System Ready: " + isReady)
       onCurrentAccountChanged: console.log("Current account set to " + currentAccount)
    }

    NavigationPanel {
        id: navDrawer
        Connections {
            target: votingSystem
            onIsReadyChanged: {
                if (votingSystem.isReady)
                    votingSystem.adaptor.getMyAccounts().then(function(accounts) {
                        navDrawer.accountList = accounts
                    })
            }
        }
    }

    FeedPage {
        id: feedPage
        backAction: navDrawer.action
    }
}

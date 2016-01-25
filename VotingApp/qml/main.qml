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
    licenseKey: "C169F952253269837F5604FE99EE6CD85C2C3604457C1626830BFDBCBD9BB390F75539977C2A4918FD5B62F4D4F236F62FB90BDD8E059F65C9CF04D5848E2597339ECC96C10945E2623372CA29F961C8B93E296FDF9C654F8BD21D3ACCC46128E2145815507281DC6C17D4810D39103B0891C41FC340FF6CC6122512D6CDDC4F460659BA576ECE24E1A40366DA5B6308CCEF71EB0A7A93F52F788EDF569A8484C60169F6792D3407DF91C435A9DCA407539523938DA57D970780F76F7FFE10713D9670FE8E8510EFB66C10E682AC81D2EE2801C3856C835F351F6C55C0CF3AFC7D3C02AC367F8BF4C4B4DDB8EB1BB7CBE410F0B730AA597543A390B6079FC77443D911CA3FE65195288F80D28AC90666098E9D49FD7C60DEC733B1D886586281401B800B0B75DE96DB19F89351AF0171"

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

            FeedPage {
                id: feedPage
                Connections {
                    target: votingSystem
                    onConnected: loadContests()
                }
            }
        }
        NavigationItem {
            title: "Coin List"

            FeedPage {
                id: coinListPage
            }
        }
    }
}

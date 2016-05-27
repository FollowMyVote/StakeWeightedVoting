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
import QtQuick.Window 2.0

import QtQmlTricks.UiElements 2.0

import VPlayApps 1.0

import FollowMyVote.StakeWeightedVoting 1.0

Page {
    id: coinListPage
    title: qsTr("Coin List")


    property VotingSystem votingSystem

    TableView {
        id: tableView
        anchors.fill: parent
        sortIndicatorVisible: true

        TableViewColumn {
            id: coinColumn
            title: "Coin"
            role: 'name'
            width: window.dp(150)
            resizable: false

            delegate: RowLayout {
                id: coinLayout

                spacing: window.dp(12)
                Item { Layout.preferredHeight: 1; Layout.preferredWidth: window.dp(8) }
                RoundedImage {

                    Layout.preferredWidth: window.dp(30)
                    Layout.preferredHeight: window.dp(30)
                    source: "qrc:/qml/res/Follow-My-Vote-Logo.png"
                    fillMode: Image.PreserveAspectCrop
                    radius: height / 2
                }
                AppText {
                    text: styleData.value
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
                Item { Layout.preferredHeight: 1; Layout.preferredWidth: window.dp(8) }
            }
        }
        TableViewColumn {
            id: creator
            title: "Creator"
            role: 'creator'
            width: tableView.viewport.width / 5
        }
        TableViewColumn {
            id: numActivePolls
            title: 'Active polls'
            width: tableView.viewport.width / 5
            role: 'contestCount'
        }
        TableViewColumn {
            id: numVotes
            title: 'Number of votes'
            width: tableView.viewport.width / 5
        }
        TableViewColumn {
            id: eligible
            title: 'Eligible'
            width: tableView.viewport.width / 5
            role: 'coinId'

            delegate: Icon {
                // Filter the currentAccount's balances for one in this coin
                property var balance: votingSystem.currentAccount.balances.list().filter(function(bal) {
                    return bal.coinId === styleData.value
                })[0]
                icon: balance && balance.amount > 0? IconType.checkcircleo : IconType.circleo
            }
        }

        model: votingSystem.coins
        rowDelegate: Rectangle {

            width: parent.width - window.dp(16)
            x: window.dp(8)
            height: window.dp(56);
            color: "white";

            Rectangle {
                ExtraAnchors.bottomDock: parent
                anchors.bottomMargin: 1

                height: 1
                color: "#ccc"
            }
        }
        headerDelegate: Item {
            height: textItem.implicitHeight * 2
            width: textItem.implicitWidth

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: window.dp(8)

                Icon {
                    visible: tableView.sortIndicatorVisible && styleData.column === tableView.sortIndicatorColumn
                    rotation: tableView.sortIndicatorOrder === Qt.AscendingOrder ? 180 : 0
                    icon: IconType.sortasc
                }
                Text {
                    id: textItem
                    Layout.fillWidth: true
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: styleData.textAlignment
                    text: styleData.value
                    elide: Text.ElideRight
                    renderType: Text.NativeRendering
                }
            }

            Rectangle {
                ExtraAnchors.bottomDock: parent
                anchors.bottomMargin: 1

                height: 2
                color: "#ccc"
            }
        }
    }
}













//ListPage {
//    id: coinListPage
//    title: qsTr("Coin List")
//    listView.header: RowLayout {
//        AppText {
//            text: "Coin"
//        }
//        AppText {
//            text: "Creator"
//        }
//        AppText {
//            text: "Number of Active Polls"
//        }
//        AppText {
//            text: "Number of Votes"
//        }
//        AppText {
//            text: "Eligible"
//        }
//    }

//    model: votingSystem.chain.coins
//    delegate: RowLayout {

//        spacing: window.dp(16)

//        RoundedImage {
//            Layout.preferredWidth: window.dp(40)
//            Layout.preferredHeight: window.dp(40)
//            source: "qrc:/qml/res/Follow-My-Vote-Logo.png"
//            fillMode: Image.PreserveAspectCrop
//            radius: height / 2
//        }
//        RowLayout{
//            ColumnLayout{

//                AppText {
//                    text: name
//                }
//                AppText {
//                    text: creator
//                }
//            }
//            AppText {
//                text: name
//            }
//        }
//    }

//     listView.spacing: window.dp(16)
//     listView.leftMargin: window.dp(16)
//     listView.bottomMargin: window.dp(16)
//     listView.topMargin: window.dp(16)

//    property VotingSystem votingSystem

// ListModel {
//     id: listModel
//     ListElement {

//     }
// }
//}

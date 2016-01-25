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
import QtQuick.Window 2.0

import VPlayApps 1.0

import FollowMyVote.StakeWeightedVoting 1.0

Page {
    id: feedPage
    title: qsTr("All Polls")

    function reloadContests() { contestList.reloadContests() }
    function loadContests() { contestList.loadContests() }

//    actions: [
//        Action {
//            name: qsTr("Find Polls")
//            iconName: "action/search"
//        },
//        Action {
//            name: qsTr("Notifications")
//            iconName: "social/notifications_none"
//        },
//        Action {
//            name: qsTr("Reload Polls")
//            iconName: "navigation/refresh"
//            onTriggered: reloadContests()
//        },
//        Action {
//            name: qsTr("Settings")
//            iconName: "action/settings"
//        },
//        Action {
//            name: qsTr("Help")
//            iconName: "action/help"
//        },
//        Action {
//            name: qsTr("Send Feedback")
//            iconName: "action/feedback"
//        }
//    ]

//    ContestantDetailDialog {
//        id: contestantDetailDialog
//        width: feedPage.width * .75
//    }
    ListModel {
        id: contestList

        property var contestGenerator

        function reloadContests() {
            contestGenerator = null
            contestList.clear()
            loadContests()
        }
        function loadContests() {
            if (!contestGenerator) {
                console.log("Setting contest generator")
                contestGenerator = votingSystem.backend.getFeedGenerator()
            }

            contestGenerator.getContests(3).then(function (contests) {
                contests.forEach(function(contest) {
                    contestList.append(contest)
                })
                if(contests.length < 3) list.footer = noMoreContestsComponent
            })
        }
    }

    ListView {
        id: list
        anchors.fill: parent
        anchors.topMargin: window.dp(8)
        anchors.bottomMargin: window.dp(8)
        model: contestList
        delegate: ContestCard {
            contestId: model.contestId
            votingStake: model.votingStake
            tracksLiveResults: model.tracksLiveResults
            onSelected: feedPage.push(Qt.createComponent(Qt.resolvedUrl("ContestPage.qml")), {"contest": contest})
        }
        spacing: window.dp(8)

        PullToRefresh {
            view: parent
            onTriggered: contestList.reloadContests()
            text: fullyPulled? qsTr("Release to Refresh") : qsTr("Pull to Refresh")
        }

        onAtYEndChanged: {
            if(list.atYEnd && votingSystem.isReady) {
                contestList.loadContests()
            }
        }
        onCountChanged: if (contentHeight < height && votingSystem.isReady)
                            contestList.loadContests()

        Component {
            id: noMoreContestsComponent
            Item {
                id: noMoreContestsFooter
                width: parent.width
                height: noMoreContests.height*3

                AppText {
                    id: noMoreContests
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    text: "There are no more contests."

                }
            }
        }
    }
//    LoadingIndicator {
//        anchors.centerIn: parent
//        height: parent.height / 7
//        visible: contestList.count === 0
//        text: qsTr("Loading Polls")
//    }
}

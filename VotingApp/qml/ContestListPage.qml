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

ListPage {
    id: contestListPage
    title: qsTr("All Polls")
    pullToRefreshHandler.enabled: true
    pullToRefreshHandler.onRefresh:{
        reloadContests()
    }

    property var getContestGeneratorFunction

    function reloadContests() { contestList.reloadContests() }
    function loadContests() {
        console.log("Loading contests...")
        contestList.loadContests()
    }

    model: contestList
    delegate: ContestCard {
        contestObject: model.contestObject
        votingStake: model.votingStake
        tracksLiveResults: model.tracksLiveResults
        onSelected: {
            feedPage.navigationStack.push(Qt.createComponent(Qt.resolvedUrl("ContestPage.qml")), {"contest": contest})
        }
    }

    listView.spacing: window.dp(8)
    listView.onAtYEndChanged: {
        if(listView.atYEnd && votingSystem.isReady) {
            contestList.loadContests()
        }
    }
    listView.onCountChanged: if (listView.contentHeight < height && votingSystem.isReady)
                                 contestList.loadContests()


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
                contestGenerator = getContestGeneratorFunction()
            }

            contestGenerator.getContests(3).then(function (contests) {
                contests.forEach(function(contest) {
                    votingSystem.adaptor.getContest(contest.contestId).then(function(contestObject) {
                        contest.contestObject = contestObject
                        contestList.append(contest)
                    }, function(error) {
                        console.log("Error when loading contest %1:\n%2".arg(contest.contestId).arg(error))
                    })
                })
                if(contests.length < 3) listView.footer = noMoreContestsComponent
            })
        }
    }
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
                text: qsTr("There are no more contests.")
            }
        }
    }

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
}

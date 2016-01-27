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
import QtGraphicalEffects 1.0

import FollowMyVote.StakeWeightedVoting 1.0

Rectangle {
    id: card
    width: parent.width - window.dp(16)
    x: window.dp(8)
    height: visibleContestLoader.height + window.dp(32)
    layer.enabled: showDropShadow
    layer.effect: DropShadow {
        radius: 4
        samples: 16
        source: card
        color: Qt.rgba(0, 0, 0, 0.5)
        transparentBorder: true
    }

    property string contestId
    property int votingStake
    property bool tracksLiveResults
    property bool showDropShadow: true

    signal selected(Contest contest)

    Behavior on height { NumberAnimation { easing.type: Easing.InOutQuad } }

    Component.onCompleted: votingSystem.adaptor.getContest(contestId).then(function(contest) {
        visibleContestLoader.setSource("ContestDelegate.qml", {"displayContest": contest})
    }, function(message) {
        console.log("Failed to load contest: " + message)
        height = 0
        visible = false
    })

    Component {
        id: loadingPlaceholder

        Item {
            height: window.dp(120)

            LoadingIndicator {
                anchors.centerIn: parent
                text: qsTr("Loading Contest")
                height: parent.height * .75
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        enabled: visibleContestLoader.status === Loader.Ready
        onClicked: card.selected(visibleContestLoader.item.displayContest)
        z: -1
    }
    Loader {
        id: visibleContestLoader
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: window.dp(16)
        }
        sourceComponent: loadingPlaceholder
    }
}

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
    x: window.dp(8)
    height: contestDelegate.height + window.dp(32)
    layer.enabled: showDropShadow
    layer.effect: DropShadow {
        radius: 2
        samples: 16
        source: card
        color: Qt.rgba(0, 0, 0, 0.5)
        transparentBorder: true
    }

    property VotingSystem votingsystem
    property int votingStake
    property bool tracksLiveResults
    property var contestObject
    property bool showDropShadow: true

    signal selected(Contest contest)

    MouseArea {
        anchors.fill: parent
        onClicked: card.selected(contestObject)
        z: -1
    }
    ContestDelegate {
        id: contestDelegate
        displayContest: contestObject
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: window.dp(16)
        }
        onCastButtonClicked: {
            votingSystem.castCurrentDecision(displayContest)
        }
        onCancelButtonClicked: {
            votingSystem.cancelCurrentDecision(displayContest)
        }
    }
}

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

import QtQmlTricks.UiElements 2.0

import FollowMyVote.StakeWeightedVoting 1.0

Rectangle {
    id: card
    x: window.dp(16)
    height: contestDelegate.y + contestDelegate.height + window.dp(16)

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
        ExtraAnchors.topDock: parent
        anchors.margins: window.dp(16)
        onCastButtonClicked: votingSystem.castCurrentDecision(displayContest)
        onCancelButtonClicked: votingSystem.cancelCurrentDecision(displayContest)
    }
}

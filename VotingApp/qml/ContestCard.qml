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

import Material 0.1
import Material.Extras 0.1
import Material.ListItems 0.1

import FollowMyVote.StakeWeightedVoting 1.0

/*
 * This type is intended to be used as a delegate. It expects the model to define three values:
 * contestId - the ID of the contest to display (hex-encoded string)
 * votingStake - the number of tokens which have specified a decision on this contest (integer)
 * tracksLiveResults - whether server has live results for this contest or not (boolean)
 */
Card {
    id: card
    width: parent.width - Units.dp(16)
    x: Units.dp(8)
    height: visibleContestLoader.height + Units.dp(32)

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
            height: Units.dp(120)

            LoadingIndicator {
                anchors.centerIn: parent
                text: qsTr("Loading Contest")
                height: parent.height * .75
            }
        }
    }

    Loader {
        id: visibleContestLoader
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: Units.dp(16)
        }
        sourceComponent: loadingPlaceholder
    }
}

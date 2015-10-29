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

import FollowMyVote.StakeWeightedVoting 1.0

Dialog {
    id: contestantDetailDialog
    hasActions: false

    Label {
        id: contestantDialogDescription
        width: parent.width
        style: "dialog"
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
    }
    Button {
        anchors.right: parent.right
        text: qsTr("Close")
        onClicked: contestantDetailDialog.close()
    }

    function display(title, description) {
        contestantDetailDialog.title = title
        contestantDialogDescription.text = description
        show()
    }
}

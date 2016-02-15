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
    id: coinListPage
    title: qsTr("Coin List")
    model: votingSystem.coins
    delegate: RowLayout {

        spacing: window.dp(16)

        RoundedImage {
            Layout.preferredWidth: window.dp(40)
            Layout.preferredHeight: window.dp(40)
            source: "qrc:/qml/res/Follow-My-Vote-Logo.png"
            fillMode: Image.PreserveAspectCrop
            radius: height / 2
        }
        ColumnLayout{

            AppText {
                text: name
            }
            AppText {
                text: qsTr("%1 contests/month").arg("N")
            }
        }
    }

     listView.spacing: window.dp(16)
     listView.leftMargin: window.dp(16)
     listView.bottomMargin: window.dp(16)
     listView.topMargin: window.dp(16)

    property VotingSystem votingSystem
}

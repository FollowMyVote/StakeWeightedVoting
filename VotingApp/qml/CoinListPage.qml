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

Page {
    id: coinListPage
    title: qsTr("Coin List")
    actionBar.maxActionCount: 3

    function loadCoins() { coinList.loadCoins() }

    ListModel {
        id: coinList

        function loadCoins() {
            votingSystem.adaptor.listAllCoins().then(function (coins) {
                coins.forEach(function(coin) {
                    coinList.append(coin)
                })
            })
        }
    }

    ListView {
        id: list
        anchors.fill: parent
        anchors.topMargin: Units.dp(8)
        anchors.bottomMargin: Units.dp(8)
        model: coinList
        delegate: Subtitled {
            text: name
            subText: qsTr("%1 contests/month").arg("N")
            iconSource: "qrc:/res/Follow-My-Vote-Logo.png"
        }

        spacing: Units.dp(8)

    }
    LoadingIndicator {
        anchors.centerIn: parent
        height: parent.height / 7
        visible: coinList.count === 0
        text: qsTr("Loading Coins")
    }
}

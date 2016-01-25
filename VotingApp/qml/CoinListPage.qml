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
    model: coinList
    delegate: AppText {
        text: name
        //subText: qsTr("%1 contests/month").arg("N")
        //iconSource: "qrc:/res/Follow-My-Vote-Logo.png"
    }

    function loadCoins() {
        console.log("Loading coins...")
        coinList.loadCoins()
    }

    ListModel {
        id: coinList

        function loadCoins() {
            votingSystem.adaptor.listAllCoins().then(function (coins) {
                console.log("Coins: " + JSON.stringify(coins))
                coins.forEach(function(coin) {
                    coinList.append(coin)
                })
            })
        }
    }
}

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

import Material 0.1
import Material.ListItems 0.1

import FollowMyVote.StakeWeightedVoting 1.0

NavigationDrawer {
    id: navDrawer
    enabled: true

    // Output properties
    property alias selectedAccountIndex: accountMenu.selectedIndex

    // Input properties
    property alias accountList: accountMenu.model

    signal navigationPageSelected(var pageName)

    Flickable {
        anchors.fill: parent

        Column {
            width: parent.width

            SimpleMenu {
                id: accountMenu
                text: qsTr("Account")

                Icon {
                    anchors.right: parent.right
                    anchors.rightMargin: Units.dp(16)
                    anchors.verticalCenter: parent.verticalCenter
                    name: "navigation/arrow_drop_down"
                }
            }
            Divider{}
            Standard {
                text: qsTr("All Polls")
                Rectangle {
                    anchors.fill: parent
                    color: Theme.alpha(Theme.tabHighlightColor, .5)
                    z: -1
                }
            }
            Standard {
                text: qsTr("Voted Polls")
            }
            Standard {
                text: qsTr("Created Polls")
            }
            Divider{}
            Standard {
                text: qsTr("Coin List")
                onClicked: {
                    navDrawer.close()
                    navigationPageSelected("coinlist")
                }
            }
        }
    }
}

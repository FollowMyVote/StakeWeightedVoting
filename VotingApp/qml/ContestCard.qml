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

import Qt.labs.settings 1.0

import Material 0.1
import Material.Extras 0.1
import Material.ListItems 0.1

import FollowMyVote.StakeWeightedVoting 1.0

Card {
    width: parent.width - Units.dp(16)
    x: Units.dp(8)
    height: contestFooter.y + contestFooter.height + Units.dp(32)
    property variant displayContest: {
        // This will be set by the Component.onCompleted handler below; set it to placeholders until then
        return {
            description: qsTr("Loading"),
            tags: {category:qsTr("Loading")},
            name: qsTr("Loading"),
            startTime: Date(),
            contestants: []
        }
    }

    property variant contestDecision: {
        if (displayContest)
            return displayContest.currentDecision
    }

    Component.onCompleted: votingSystem.adaptor.getContest(contestId).then(function(contest) {
        displayContest = contest
    })

    Column {
        anchors.fill: parent
        anchors.margins: Units.dp(16)
        spacing: Units.dp(16)

        RowLayout {
            id: categoryLayout
            width: parent.width

            Label {
                Layout.fillWidth: true
                text: displayContest.tags["category"]
                style: "button"
                color: Theme.accentColor
            }
            Icon {
                name: "hardware/cast"
                color: Theme.accentColor
            }
        }
        Rectangle { height: Units.dp(1); width: parent.width; color: "lightgrey" }
        GridLayout {
            id: contestLayout
            anchors {
                left: parent.left
                right: parent.right
            }
            columns: width > 120 * Units.pixelDensity && displayContest.description.length < 1000? 2 : 1
            columnSpacing: Units.dp(16)

            ColumnLayout {
                id: contestDetailsLayout
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                spacing: 0

                RowLayout {
                    id: contestHeader
                    spacing: Units.dp(16)
                    height: Units.dp(72)
                    Layout.fillWidth: true

                    CircleImage {
                        id: coinImage
                        width: Units.dp(40)
                        height: Units.dp(40)
                        source: "https://followmyvote.com/wp-content/uploads/2014/02/Follow-My-Vote-Logo.png"
                        fillMode: Image.PreserveAspectCrop
                    }
                    Column {
                        Layout.alignment: Qt.AlignVCenter
                        Layout.fillWidth: true
                        Label {
                            width: parent.width
                            style: "title"
                            text: displayContest.name
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        }
                        Label {
                            width: parent.width
                            style: "subheading"
                            text: displayContest.startTime.toLocaleString(Qt.locale(), Locale.ShortFormat)
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            opacity: .54
                        }
                    }
                }
                Item { width: 1; height: Units.dp(24) }
                Label {
                    text: displayContest.description
                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
            }
            GridLayout {
                id: contestantGrid
                columns: Math.max(1, Math.floor(width / contestantMinimumWidth()))
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                columnSpacing: Units.dp(8)
                rowSpacing: columnSpacing

                function contestantMinimumWidth() {
                    switch (Device.type) {
                    case Device.phone:
                        return Units.pixelDensity * 50
                    case Device.desktop:
                        return Units.pixelDensity * 50
                    }
                }

                Repeater {
                    id: contestantRepeater
                    model: displayContest.contestants
                    delegate: Card {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.preferredHeight: {
                            var maxHeight = 0
                            for (var i = 0; i < contestantRepeater.count; ++i)
                                if (contestantRepeater.itemAt(i) !== null)
                                    maxHeight = Math.max(maxHeight, contestantRepeater.itemAt(i).contentHeight)
                            return maxHeight + Units.dp(16)
                        }
                        tintColor: contestDecision && contestDecision.opinions[index]? Theme.accentColor : "transparent"

                        Behavior on tintColor {
                            ColorAnimation {
                                duration: 100
                            }
                        }

                        property alias contentHeight: contestantColumn.height

                        Ink {
                            z: 0
                            anchors.fill: parent
                            onClicked: {
                                var opinions = {}
                                opinions[index] = 1
                                contestDecision.opinions = opinions
                            }
                        }
                        ColumnLayout {
                            id: contestantColumn
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.margins: Units.dp(8)
                            y: Units.dp(8)

                            Label {
                                style: "title"
                                text: modelData.name
                                Layout.fillWidth: true
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            }
                            Label {
                                id: contestantDescription
                                text: modelData.description
                                Layout.fillWidth: true
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                elide: Text.ElideRight
                                maximumLineCount: 5
                            }
                            Button {
                                text: qsTr("Read more")
                                visible: contestantDescription.truncated
                                Layout.fillWidth: true
                                onClicked: contestantDetailDialog.display(modelData.name, modelData.description)
                            }
                        }
                    }
                }
            }
        }
        Rectangle { height: Units.dp(1); width: parent.width; color: "lightgrey" }
        RowLayout {
            id: contestFooter
            anchors {
                left: parent.left
                right: parent.right
            }

            Button {
                text: qsTr("Cast Vote")
                onClicked: {
                    if (votingSystem.castPendingDecision(displayContest)) {
                    }
                }
            }
            Button {
                text: qsTr("Cancel")
                onClicked: {
                    votingSystem.clearPendingDecision(contestId)
                }
            }
            Item { height: 1; Layout.fillWidth: true }
            IconButton {
                iconName: "social/share"
            }
        }
    }
}

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
Column {
    spacing: Units.dp(16)

    property var displayContest

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
                    source: "res/Follow-My-Vote-Logo.png"
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
                    tintColor: displayContest.currentDecision &&
                               displayContest.currentDecision.opinions[index]? Theme.accentColor : "transparent"

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
                            displayContest.currentDecision.opinions = opinions
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

        Row {
            Button {
                text: qsTr("Cast Vote")
                visible: displayContest.currentDecision && displayContest.currentDecision.state === Decision.Pending
                onClicked: {
                    votingSystem.castCurrentDecision(displayContest)
                }
            }
            Button {
                text: qsTr("Cancel")
                onClicked: {
                    votingSystem.adaptor.getDecision(votingSystem.currentAccount,
                                                     contestId).then(function(decision) {
                                                         displayContest.currentDecision = decision
                                                     }, function() {
                                                         console.log("No decision found, resetting opinions instead. " +
                                                                     "The following error can be ignored.")
                                                         displayContest.currentDecision.opinions = []
                                                         displayContest.currentDecision.state = Decision.Pending
                                                     })
                }
            }
        }
        Item { height: 1; Layout.fillWidth: true }
        IconButton {
            iconName: "social/share"
        }
    }
}

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0

import VPlayApps 1.0

import FollowMyVote.StakeWeightedVoting 1.0

Column {
    spacing: window.dp(16)

    property var displayContest

    RowLayout {
        id: categoryLayout
        width: parent.width

        AppText {
            Layout.fillWidth: true
            text: displayContest.tags["category"]
            color: Theme.textColor
        }
        Icon {
            icon: IconType.wifi
            color: Theme.tintColor
        }
    }
    Rectangle { height: window.dp(1); width: parent.width; color: "lightgrey" }
    GridLayout {
        id: contestLayout
        anchors {
            left: parent.left
            right: parent.right
        }
        columns: width > 120 * window.pixelDensity && displayContest.description.length < 1000? 2 : 1
        columnSpacing: window.dp(16)

        ColumnLayout {
            id: contestDetailsLayout
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            spacing: 0

            RowLayout {
                id: contestHeader
                spacing: window.dp(16)
                height: window.dp(72)
                Layout.fillWidth: true

                AppImage {
                    id: coinImage
                    Layout.preferredWidth: window.dp(40)
                    Layout.preferredHeight: window.dp(40)
                    source: "res/Follow-My-Vote-Logo.png"
                    fillMode: Image.PreserveAspectCrop
                }
                Column {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.fillWidth: true
                    AppText {
                        width: parent.width
                        text: displayContest.name
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    }
                    AppText {
                        width: parent.width
                        text: displayContest.startTime.toLocaleString(Qt.locale(), Locale.ShortFormat)
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        opacity: .54
                    }
                }
            }
            Item { width: 1; height: window.dp(24) }
            AppText {
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
            columnSpacing: window.dp(8)
            rowSpacing: columnSpacing

            function contestantMinimumWidth() {
                return window.pixelDensity * 50
            }

            Repeater {
                id: contestantRepeater
                model: displayContest.contestants
                delegate: Rectangle {
                    id: contestantButton
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredHeight: {
                        var maxHeight = 0
                        for (var i = 0; i < contestantRepeater.count; ++i)
                            if (contestantRepeater.itemAt(i) !== null)
                                maxHeight = Math.max(maxHeight, contestantRepeater.itemAt(i).contentHeight)
                        return maxHeight + window.dp(16)
                    }
                    color: isSelected? Theme.tintColor : Theme.tintLightColor
                    opacity: isSelected? 1 : .5

                    Behavior on color { ColorAnimation { easing.type: Easing.OutQuad } }
                    Behavior on opacity { NumberAnimation { easing.type: Easing.OutQuad } }

                    property bool isSelected: !!displayContest.currentDecision &&
                                              !!displayContest.currentDecision.opinions[index]

                    property alias contentHeight: contestantColumn.height

                    MouseArea {
                        z: 0
                        anchors.fill: parent
                        onClicked: {
                            var opinions = {}
                            opinions[index] = !isSelected
                            displayContest.currentDecision.opinions = opinions
                        }
                    }
                    ColumnLayout {
                        id: contestantColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: window.dp(8)
                        y: window.dp(8)

                        AppText {
                            text: modelData.name
                            Layout.fillWidth: true
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            color: contestantButton.isSelected? "white" : "black"
                            font.weight: Font.DemiBold
                        }
                        AppText {
                            id: contestantDescription
                            text: modelData.description
                            Layout.fillWidth: true
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            elide: Text.ElideRight
                            maximumLineCount: 5
                            color: contestantButton.isSelected? "white" : "black"
                        }
                        AppButton {
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
    Rectangle { height: window.dp(1); width: parent.width; color: "lightgrey" }
    RowLayout {
        id: contestFooter
        anchors {
            left: parent.left
            right: parent.right
        }

        Row {
            AppButton {
                text: qsTr("Cast Vote")
                visible: displayContest.currentDecision && displayContest.currentDecision.state === Decision.Pending
                onClicked: {
                    votingSystem.castCurrentDecision(displayContest)
                }
            }
            AppButton {
                text: qsTr("Cancel")
                onClicked: {
                    votingSystem.adaptor.getDecision(votingSystem.currentAccount,
                                                     displayContest.id).then(function(decision) {
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
        Icon {
            icon: IconType.share
        }
    }
}

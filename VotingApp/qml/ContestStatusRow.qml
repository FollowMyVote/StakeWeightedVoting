import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.1
import QtQml 2.2
import QtGraphicalEffects 1.0
import Qt.labs.settings 1.0
import QtQmlTricks.UiElements 2.0 as UI
import FollowMyVote.StakeWeightedVoting 1.0
import QuickPromise 1.0

Item {
    id: statusRowContainer
    clip: true
    state: "HIDDEN"

    Behavior on height { NumberAnimation {
            duration: 150
            easing.type: Easing.InOutQuad
        } }
    
    RowLayout {
        id: statusRow
        width: parent.width
        spacing: 8
        
        UI.SvgIconLoader {
            id: statusIcon
            Layout.fillHeight: true
            size: height
        }
        Label {
            id: statusText
            font.pixelSize: statusIcon.height / 2
            font.weight: Font.DemiBold
        }
        Item { /* spacer */ Layout.preferredHeight: 1; Layout.fillWidth: true }
        Row {
            id: buttonRow
            height: childrenRect.height
            spacing: parent.spacing
            clip: true

            property real pendingWidth: cancelButton.width + spacing + castButton.width
            property real officialWidth: revokeButton.width

            Button {
                id: cancelButton
                text: qsTr("Cancel")
                onClicked: votingSystem.cancelPendingDecision(contest)
            }
            Button {
                id: castButton
                text: qsTr("Cast Vote")
                onClicked: votingSystem.castPendingDecision(contest)
            }
            Button {
                id: revokeButton
                text: qsTr("Revoke Vote")
                onClicked: {
                    contest.pendingDecision.opinions = {}
                    contest.pendingDecision.writeIns = []
                    votingSystem.castPendingDecision(contest)
                }
            }
        }
    }

    states: [
        State {
            name: "HIDDEN"
            PropertyChanges {
                target: statusRowContainer
                height: 0
            }
            PropertyChanges {
                target: cancelButton
                visible: true
            }
            PropertyChanges {
                target: castButton
                visible: true
            }
            PropertyChanges {
                target: revokeButton
                visible: false
            }
            PropertyChanges {
                target: statusText
                text: qsTr("Pending")
                color: Material.color(Material.Grey)
            }
            PropertyChanges {
                target: statusIcon
                icon: "qrc:/icons/alert/warning.svg"
                color: Material.color(Material.Grey)
            }
        },
        State {
            name: "SHOW_PENDING"
            PropertyChanges {
                target: statusRowContainer
                height: statusRow.height
            }
            PropertyChanges {
                target: cancelButton
                visible: true
            }
            PropertyChanges {
                target: castButton
                visible: true
            }
            PropertyChanges {
                target: revokeButton
                visible: false
            }
            PropertyChanges {
                target: statusText
                text: qsTr("Pending")
                color: Material.color(Material.Grey)
            }
            PropertyChanges {
                target: statusIcon
                icon: "qrc:/icons/alert/warning.svg"
                color: Material.color(Material.Grey)
            }
        },
        State {
            name: "SHOW_OFFICIAL"
            PropertyChanges {
                target: statusRowContainer
                height: statusRow.height
            }
            PropertyChanges {
                target: cancelButton
                visible: false
            }
            PropertyChanges {
                target: castButton
                visible: false
            }
            PropertyChanges {
                target: revokeButton
                visible: true
            }
            PropertyChanges {
                target: statusText
                text: qsTr("Official")
                color: Material.foreground
            }
            PropertyChanges {
                target: statusIcon
                icon: "qrc:/icons/action/check_circle.svg"
                color: Material.color(Material.Green)
            }
        }
    ]
    transitions: [
        Transition {
            from: "SHOW_PENDING"
            to: "SHOW_OFFICIAL"
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation {
                        target: buttonRow
                        duration: 150
                        easing.type: Easing.InOutQuad
                        property: "Layout.preferredWidth"
                        from: buttonRow.pendingWidth; to: 0
                    }
                    NumberAnimation {
                        targets: [statusIcon, statusText]
                        property: "opacity"
                        duration: 100
                        easing.type: Easing.Linear
                        from: 1; to: 0
                    }
                }
                PropertyAction { target: statusIcon; properties: "icon,color" }
                PropertyAction { target: statusText; properties: "text,color" }
                PropertyAction { targets: [cancelButton, castButton, revokeButton]; properties: "visible" }
                PauseAnimation { duration: 50 }
                ParallelAnimation {
                    NumberAnimation {
                        target: buttonRow
                        duration: 150
                        easing.type: Easing.InOutQuad
                        property: "Layout.preferredWidth"
                        to: buttonRow.officialWidth; from: 0
                    }
                    NumberAnimation {
                        targets: [statusIcon, statusText]
                        property: "opacity"
                        duration: 100
                        easing.type: Easing.Linear
                        to: 1; from: 0
                    }
                }
            }
        },
        Transition {
            from: "SHOW_OFFICIAL"
            to: "SHOW_PENDING"
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation {
                        target: buttonRow
                        duration: 150
                        easing.type: Easing.InOutQuad
                        property: "Layout.preferredWidth"
                        from: buttonRow.officialWidth; to: 0
                    }
                    NumberAnimation {
                        targets: [statusIcon, statusText]
                        property: "opacity"
                        duration: 100
                        easing.type: Easing.Linear
                        from: 1; to: 0
                    }
                }
                PropertyAction { target: statusIcon; properties: "icon,color" }
                PropertyAction { target: statusText; properties: "text,color" }
                PropertyAction { targets: [cancelButton, castButton, revokeButton]; properties: "visible" }
                PauseAnimation { duration: 50 }
                ParallelAnimation {
                    NumberAnimation {
                        target: buttonRow
                        duration: 150
                        easing.type: Easing.InOutQuad
                        property: "Layout.preferredWidth"
                        to: buttonRow.pendingWidth; from: 0
                    }
                    NumberAnimation {
                        targets: [statusIcon, statusText]
                        property: "opacity"
                        duration: 100
                        easing.type: Easing.Linear
                        to: 1; from: 0
                    }
                }
            }
        }
    ]
}

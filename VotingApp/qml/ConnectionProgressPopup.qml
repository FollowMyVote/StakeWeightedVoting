import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import Qt.labs.settings 1.0
import QtQmlTricks.UiElements 2.0 as UI
import FollowMyVote.StakeWeightedVoting 1.0

Popup {
    closePolicy: Popup.NoAutoClose
    modal: true
    x: window.width / 2 - width / 2
    y: window.height / 2 - height / 2
    z: 3
    background: Rectangle {}
    enter: Transition {
        PropertyAnimation {
            targets: [contentItem, background]
            property: "opacity"
            from: 0; to: 1
            duration: 250
        }
    }
    exit: Transition {
        PropertyAnimation {
            targets: [contentItem, background]
            property: "opacity"
            from: 1; to: 0
            duration: 250
        }
    }

    property alias progress: progressBar.value
    property alias text: progressLabel.text

    ColumnLayout {
        ProgressBar {
            id: progressBar
            indeterminate: value === 0
            Behavior on value {
                NumberAnimation { easing.type: Easing.InOutQuad }
            }
        }
        Label {
            id: progressLabel
            Layout.alignment: Qt.AlignHCenter
        }
    }
}

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import Qt.labs.settings 1.0

import VPlayApps 1.0

import FollowMyVote.StakeWeightedVoting 1.0

Rectangle {
    id: createContestButton

    //Intentionally unusual numbers to make up for border width
    width: parent.width - window.dp(28)
    x: window.dp(14)
    z: 5

    border.width: window.dp(2)
    border.color: "lightgrey"

    height: window.dp(100)
    signal clicked

    AppText {
        anchors.centerIn: parent
        text: qsTr("Create Poll")
        font.pixelSize: window.sp(24)
        font.weight: Font.Bold

    }
    MouseArea {
        anchors.fill: parent
        onClicked: createContestButton.clicked()

    }

}


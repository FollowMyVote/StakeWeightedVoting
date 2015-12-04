import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0

import Material 0.1

import FollowMyVote.StakeWeightedVoting 1.0

ColumnLayout {
    property alias text: loadingText.text
    width: height * 2

    ProgressCircle {
        anchors.horizontalCenter: parent.horizontalCenter
        Layout.fillHeight: true
        width: height
    }
    Label {
        id: loadingText
        anchors.horizontalCenter: parent.horizontalCenter
        Layout.fillWidth: true
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        horizontalAlignment: Text.AlignHCenter
    }
}

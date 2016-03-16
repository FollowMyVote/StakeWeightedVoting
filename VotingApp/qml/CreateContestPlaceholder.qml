import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import Qt.labs.settings 1.0
import "CustomControls"
import VPlayApps 1.0

import FollowMyVote.StakeWeightedVoting 1.0


Rectangle {

    id: createContestBar

    color: "white"
    width: parent.width - window.dp(28)
    height: createContestButton.height + window.dp(16)
    x: window.dp(14)
    z: 5

    border.width: window.dp(2)
    border.color: "lightgrey"

    property VotingSystem votingSystem

    AppTextField {
        id: search
        placeholderText: qsTr("Search")
        anchors.left: createContestBar.left
        anchors.leftMargin: window.dp(16)
        anchors.verticalCenter: createContestBar.verticalCenter
        anchors.margins: window.dp(8)
        width: window.dp(200)
        height: window.dp(36)
        borderWidth: window.dp(2)
        radius: 5
    }


    AppButton {
        id: createContestButton
        anchors.right: createContestBar.right
        anchors.top: createContestBar.top
        anchors.margins: window.dp(8)

        onClicked: {
            console.log("Begin contest creation")
            myContestsPage.navigationStack.push(Qt.resolvedUrl("CreateContestPage.qml"),
                                                {"contestCreator": votingSystem.backend.contestCreator,
                                                    "votingSystem": votingSystem})
        }

        RowLayout {
            spacing: window.dp(8)
            anchors.centerIn: parent
            onWidthChanged: createContestButton.implicitContentWidth = width
            onHeightChanged: createContestButton.implicitContentHeight = height

            Icon {
                id: innerIcon
                icon: IconType.plus
                color: "white"
            }
            AppText {
                text: qsTr("Create Poll")
                color: Theme.appButton.textColor
                font.family: Theme.appButton.fontBold
                font.pixelSize: sp(Theme.appButton.textSize)
                elide: Text.ElideRight
                Layout.preferredHeight: contentHeight
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                font.weight: Font.Bold
                font.capitalization: Theme.appButton.fontCapitalization

            }
        }
    }
}

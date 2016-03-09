import QtQuick 2.5
import QtQuick.Layouts 1.1
import Qt.labs.controls 1.0
import QtQml 2.2

import VPlayApps 1.0

import QtQmlTricks.UiElements 2.0

import FollowMyVote.StakeWeightedVoting 1.0

Page {
    id: createContestPage
    backgroundColor: "white"

    property var contestCreator
    property VotingSystem votingSystem
    property var purchaseRequest: contestCreator.getPurchaseContestRequest()

    SwipeView {
        id: swiper
        anchors.fill: parent

        BasicContestForm {
            votingSystem: createContestPage.votingSystem
            purchaseRequest: createContestPage.purchaseRequest
        }
    }
    PageControl {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        pages: swiper.count
        currentPage: swiper.currentIndex
    }

    Component {
        id: contestantDialog

        Dialog {
            title: qsTr("Edit Contestant")
            contentHeight: window.dp(200)
            contentWidth: window.dp(300)

            property alias contestantName: contestantName.text
            property alias contestantDescription: contestantDescription.text

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: window.dp(8)

                AppTextField {
                    id: contestantName
                    placeholderText: qsTr("Name")
                    maximumLength: contestCreator.contestLimits[ContestLimits.ContestantNameLength]
                    Layout.fillWidth: true
                    KeyNavigation.tab: contestantDescription
                    Component.onCompleted: forceActiveFocus(Qt.Popup)
                }
                ScrollingTextEdit {
                    id: contestantDescription
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    textEdit.placeholderText: qsTr("Description")
                }
            }
        }
    }
}

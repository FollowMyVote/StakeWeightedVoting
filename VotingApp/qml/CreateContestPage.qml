import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4 as Controls
import QtQuick.Controls.Styles 1.4 as ControlStyles
import Qt.labs.controls 1.0

import VPlayApps 1.0

import FollowMyVote.StakeWeightedVoting 1.0

Page {
    id: createContestPage
    backgroundColor: "white"

    property var contestCreator
    property VotingSystem votingSystem

    SwipeView {
        id: swiper
        anchors.fill: parent

        Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: window.dp(16)

                AppTextField {
                    Layout.fillWidth: true
                    placeholderText: qsTr("Contest Name")
                    maximumLength: contestCreator.contestLimits[ContestLimits.NameLength]
                }
                AppTextEdit {
                    Layout.fillWidth: true
                    placeholderText: qsTr("Description")
                    wrapMode: TextEdit.Wrap
                }
                Row {
                    spacing: window.dp(8)

                    AppText {
                        id: weightCoinLabel
                        text: qsTr("Coin to Poll:")
                    }
                    Controls.ComboBox {
                        width: window.dp(120)
                        model: ["a", "b", "c"]
                        style: ControlStyles.ComboBoxStyle {
                            font: weightCoinLabel.font
                        }
                    }
                }
            }
        }
    }

    PageControl {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        pages: swiper.count
        currentPage: swiper.currentIndex
    }
}

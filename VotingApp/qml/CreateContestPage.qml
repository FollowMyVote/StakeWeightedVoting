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
    property var purchaseRequest: contestCreator.getPurchaseContestRequest()

    SwipeView {
        id: swiper
        anchors.fill: parent

        Item {
            Flickable {
                id: flickable
                interactive: true
                flickableDirection: Flickable.VerticalFlick
                contentHeight: createContestFormColumn.height
                anchors {
                    left: parent.left
                    right: parent.horizontalCenter
                    top: parent.top
                    bottom: parent.bottom
                    margins: window.dp(16)
                    rightMargin: window.dp(8)
                }

                Column {
                    id: createContestFormColumn
                    width: parent.width

                    AppTextField {
                        width: parent.width
                        placeholderText: qsTr("Contest Name")
                        maximumLength: contestCreator.contestLimits[ContestLimits.NameLength]
                    }
                    AppTextEdit {
                        width: parent.width
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

                    Column {
                        width: parent.width
                        Repeater {

                            model: purchaseRequest.contestants
                            delegate: Row {
                                width: parent.width

                                IconButton {
                                    icon: IconType.edit
                                }
                                IconButton {
                                    icon: IconType.remove
                                    onClicked: {
                                        purchaseRequest.contestants.remove(index)
                                    }
                                }

                                AppText {
                                    text: modelData.name
                                    font.pixelSize: window.dp(30)
                                }
                                AppText {
                                    text: modelData.description
                                }


                            }
                        }
                    }

                    Row {
                        width : parent.width
                        spacing: window.dp(8)
                        AppTextField {
                            id: contestantName
                            placeholderText: qsTr("Contestant Name ")
                            width: window.dp(200)

                        }
                        AppTextEdit {
                            id: contestantDescription
                            placeholderText: qsTr("Contestant Description")
                            width: window.dp(200)
                            wrapMode: TextEdit.WrapAtWordBoundaryOrAnywhere
                        }

                        AppButton {
                            text: qsTr("Add Contestant")
                            width: window.dp(200)
                            onClicked: {
                                if (contestantName.text != "") {
                                    purchaseRequest.contestants.append({"name": contestantName.text, "description": contestantDescription.text})
                                    contestantName.text = ""
                                    contestantDescription.text = ""
                                }
                            }
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


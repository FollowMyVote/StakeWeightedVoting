import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4 as Controls
import QtQuick.Controls.Styles 1.4 as ControlStyles
import QtQuick.Extras 1.4 as Extras
import Qt.labs.controls 1.0
import QtQml 2.2

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
                        id: contestName
                        width: parent.width
                        placeholderText: qsTr("Contest Name")
                        maximumLength: contestCreator.contestLimits[ContestLimits.NameLength]
                        Component.onCompleted: forceActiveFocus()
                        KeyNavigation.tab: contestDescription

                        Binding {
                            target: purchaseRequest
                            property: "name"
                            value: contestName.text
                        }
                    }
                    AppTextEdit {
                        id: contestDescription
                        width: parent.width
                        placeholderText: qsTr("Description")
                        wrapMode: TextEdit.Wrap

                        Binding {
                            target: purchaseRequest
                            property: "description"
                            value: contestDescription.text
                        }
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
                            delegate: RowLayout {
                                width: parent.width

                                Row {
                                    // This row works around the fact that IconButton does not properly set implicitWidth
                                    IconButton {
                                        icon: IconType.remove
                                        onClicked: purchaseRequest.contestants.remove(index)
                                        implicitWidth: minim
                                    }
                                    IconButton {
                                        icon: IconType.edit
                                        onClicked: {
                                            var dialog = contestantDialog.createObject(createContestPage,
                                                                                       {"contestantName": name,
                                                                                           "contestantDescription":
                                                                                           description})

                                            dialog.accepted.connect(function() {
                                                name = dialog.contestantName
                                                description = dialog.contestantDescription
                                                dialog.close()
                                            })
                                            dialog.canceled.connect(dialog.close)

                                            dialog.open()
                                        }
                                    }
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    AppText {
                                        text: name
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                    }
                                    AppText {
                                        text: description
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }
                    AppButton {
                        text: qsTr("Add Contestant")

                        onClicked: {
                            // Create a new dialog as defined by the contestDialog component
                            var dialog = contestantDialog.createObject(createContestPage)

                            // Handle dialog accepted/canceled signals
                            dialog.accepted.connect(function() {
                                var contestant = Qt.createQmlObject("import FollowMyVote.StakeWeightedVoting." +
                                                                    "ContestPurchase 1.0; Contestant{}",
                                                                    createContestPage, "ContestantCreation")
                                contestant.name = dialog.contestantName;
                                contestant.description = dialog.contestantDescription
                                purchaseRequest.contestants.append(contestant)
                                dialog.close()
                            })
                            dialog.canceled.connect(dialog.close)

                            dialog.open()
                        }
                    }
                    Row {
                        spacing: window.dp(8)

                        Binding {
                            target: purchaseRequest
                            property: "expiration"
                            value: {
                                if (contestEndsTime.currentIndex === 0)
                                    return new Date(0)

                                var date = new Date()
                                if (contestEndsTime.currentIndex === 1)
                                    date.setDate(date.getDate() + 1)
                                else if (contestEndsTime.currentIndex === 2)
                                    date.setDate(date.getDate() + 7)
                                else if (contestEndsTime.currentIndex === 3)
                                    date.setMonth(date.getMonth() + 1)
                                return date
                            }
                        }

                        AppText {
                            text: qsTr("Contest Ends")
                        }
                        Controls.ComboBox {
                            id: contestEndsTime
                            width: window.dp(100)
                            model: [qsTr("never"), qsTr("in a day"), qsTr("in a week"), qsTr("in a month")]
                            style: ControlStyles.ComboBoxStyle {
                                font: weightCoinLabel.font
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

    Component {
        id: contestantDialog

        Dialog {
            title: qsTr("Edit Contestant")
            contentHeight: window.dp(200)
            contentWidth: window.dp(250)

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

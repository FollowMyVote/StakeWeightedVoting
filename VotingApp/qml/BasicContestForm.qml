import QtQuick 2.5
import QtQuick.Layouts 1.1

import VPlayApps 1.0

import QtQmlTricks.UiElements 2.0

import FollowMyVote.StakeWeightedVoting 1.0

Item {
    id: basicContestForm

    property VotingSystem votingSystem
    property var purchaseRequest
    property var contestCreator

    Flickable {
        id: flickable
        interactive: true
        flickableDirection: Flickable.VerticalFlick
        contentHeight: createContestFormColumn.height
        
        ExtraAnchors.leftDock: parent
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: window.dp(8)
        
        Column {
            id: createContestFormColumn
            width: parent.width
            spacing: window.dp(8)
            ExtraAnchors.topLeftCorner: parent
            anchors.margins: window.dp(16)
            
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
                ComboList {
                    width: window.dp(120)
                    model: votingSystem.coins
                    delegate: ComboListDelegateForModelWithRoles {
                        roleKey: "coinId"
                        roleValue: "name"
                        label.font: weightCoinLabel.font
                    }
                    Component.onCompleted: selectByKey(0)
                    
                    Binding {
                        target: purchaseRequest
                        property: "weightCoin"
                        value: parent.currentKey
                    }
                }
            }
            Column {
                width: parent.width
                spacing: window.dp(8)
                Repeater {
                    model: purchaseRequest.contestants
                    delegate: RowLayout {
                        width: parent.width
                        
                        IconButton {
                            icon: IconType.remove
                            onClicked: purchaseRequest.contestants.remove(index)
                        }
                        IconButton {
                            icon: IconType.edit
                            onClicked: {
                                var dialog = contestantDialog.createObject(basicContestForm, {
                                                                               "contestantName": name,
                                                                               "contestantDescription": description})
                                
                                dialog.accepted.connect(function() {
                                    name = dialog.contestantName
                                    description = dialog.contestantDescription
                                    dialog.close()
                                })
                                dialog.canceled.connect(dialog.close)
                                
                                dialog.open()
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
                    var dialog = contestantDialog.createObject(basicContestForm)
                    
                    // Handle dialog accepted/canceled signals
                    dialog.accepted.connect(function() {
                        var contestant = Qt.createQmlObject("import FollowMyVote.StakeWeightedVoting." +
                                                            "ContestPurchase 1.0; Contestant{}",
                                                            basicContestForm, "ContestantCreation")
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
                        if (contestEndsTime.currentIdx === 1)
                            date.setDate(date.getDate() + 1)
                        else if (contestEndsTime.currentIdx === 2)
                            date.setDate(date.getDate() + 7)
                        else if (contestEndsTime.currentIdx === 3)
                            date.setMonth(date.getMonth() + 1)
                        return date
                    }
                }
                
                AppText {
                    id: contestEndsLabel
                    text: qsTr("Contest Ends")
                }
                ComboList {
                    id: contestEndsTime
                    width: window.dp(100)
                    model: [qsTr("never"), qsTr("in a day"), qsTr("in a week"), qsTr("in a month")]
                    delegate: ComboListDelegateForSimpleVar {
                        label.font: weightCoinLabel.font
                    }
                    Component.onCompleted: currentIdx = 0
                }
            }
        }
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

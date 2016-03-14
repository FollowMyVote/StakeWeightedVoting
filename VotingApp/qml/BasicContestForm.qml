import QtQuick 2.5
import QtQuick.Layouts 1.1

import VPlayApps 1.0

import QtQmlTricks.UiElements 2.0

import FollowMyVote.StakeWeightedVoting 1.0

Item {
    id: basicContestForm

    // Input only properties
    property var contestLimits
    property alias coinsModel: weightCoin.model

    // Input/output properties
    property alias contestName: contestName.text
    property alias contestDescription: contestDescription.text

    // Output only properties
    readonly property alias contestantModel: contestantRepeater.model
    readonly property alias weightCoinId: weightCoin.currentKey
    readonly property var contestExpiration: {
        if (contestEndsTime.currentIdx === 0)
            return 0

        var date = new Date()
        if (contestEndsTime.currentIdx === 1)
            date.setDate(date.getDate() + 1)
        else if (contestEndsTime.currentIdx === 2)
            date.setDate(date.getDate() + 7)
        else if (contestEndsTime.currentIdx === 3)
            date.setMonth(date.getMonth() + 1)
        return date.getTime()
    }

    function setWeightCoinId(id) {
        weightCoin.selectByKey(id)
    }

    signal completed

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
                maximumLength: contestLimits[ContestLimits.NameLength]
                Component.onCompleted: forceActiveFocus()
                KeyNavigation.tab: contestDescription
            }
            AppTextEdit {
                id: contestDescription
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
                ComboList {
                    id: weightCoin
                    width: window.dp(120)
                    delegate: ComboListDelegateForModelWithRoles {
                        roleKey: "coinId"
                        roleValue: "name"
                        label.font: weightCoinLabel.font
                    }
                    Component.onCompleted: selectByKey(0)
                }
            }
            Column {
                width: parent.width
                spacing: window.dp(8)
                Repeater {
                    id: contestantRepeater
                    model: ListModel{}
                    delegate: RowLayout {
                        width: parent.width
                        
                        IconButton {
                            icon: IconType.remove
                            onClicked: contestantRepeater.model.remove(index)
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
                                    dialog.destroy()
                                })
                                dialog.canceled.connect(function() {
                                    dialog.close()
                                    dialog.destroy()
                                })
                                
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
                        var contestant = {"name": dialog.contestantName, "description": dialog.contestantDescription}
                        contestantRepeater.model.append(contestant)
                        dialog.close()
                    })
                    dialog.canceled.connect(dialog.close)
                    
                    dialog.open()
                }
            }
            Row {
                spacing: window.dp(8)
                
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
            AppButton {
                text: qsTr("Continue")
                onClicked: completed()
                implicitHeight: contentHeight
                implicitWidth: contentWidth
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
                    maximumLength: contestLimits[ContestLimits.ContestantNameLength]
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

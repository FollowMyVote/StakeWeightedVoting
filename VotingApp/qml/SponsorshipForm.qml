import QtQuick 2.5
import QtQuick.Layouts 1.1
import Qt.labs.controls 1.0
import QtQml 2.2

import VPlayApps 1.0

import QtQmlTricks.UiElements 2.0

import FollowMyVote.StakeWeightedVoting 1.0

Flickable {
    property alias sponsorshipEnabled: sponsorshipEnabledSwitch.checked
    property alias maxVotes: maxVotesField.text
    property alias maxRevotes: maxRevotesField.text
    readonly property var endTime: {
        if (sponsorshipEndTime.currentIdx === 0)
            return 0

        var date = new Date()
        if (sponsorshipEndTime.currentIdx === 1)
            date.setDate(date.getDate() + 1)
        else if (sponsorshipEndTime.currentIdx === 2)
            date.setDate(date.getDate() + 7)
        else if (sponsorshipEndTime.currentIdx === 3)
            date.setMonth(date.getMonth() + 1)
        return date.getTime()
    }
    property alias incentive: incentiveField.text


    flickableDirection: Flickable.VerticalFlick
    
    Column {
        ExtraAnchors.topLeftCorner: parent
        anchors.margins: window.dp(16)
        
        Row {
            AppText {
                text: qsTr("Sponsor Votes")
            }
            AppSwitch {
                id: sponsorshipEnabledSwitch
                height: window.dp(30)
            }
        }
        Column {
            enabled: sponsorshipEnabledSwitch.checked
            
            Row {
                AppText {
                    text: qsTr("Max votes to sponsor")
                }
                AppTextField {
                    id: maxVotesField
                    inputMethodHints: Qt.ImhDigitsOnly
                    validator: IntValidator { bottom: 0 }
                }
            }
            Row {
                AppText {
                    text: qsTr("Max replacement votes per voter to sponsor")
                }
                AppTextField {
                    id: maxRevotesField
                    inputMethodHints: Qt.ImhDigitsOnly
                    validator: IntValidator { bottom: 0 }
                }
            }
            Row {
                spacing: window.dp(8)
                
                Binding {
                    target: purchaseRequest
                    property: "sponsorEndDate"
                    value: {
                    }
                }
                
                AppText {
                    id: sponshorshipEndsLabel
                    text: qsTr("Sponsorship ends")
                }
                ComboList {
                    id: sponsorshipEndTime
                    width: window.dp(100)
                    model: [qsTr("never"), qsTr("in a day"), qsTr("in a week"), qsTr("in a month")]
                    delegate: ComboListDelegateForSimpleVar {
                        label.font: sponshorshipEndsLabel.font
                    }
                    Component.onCompleted: currentIdx = 0
                }
            }
            Row {
                AppText {
                    text: qsTr("Voter incentive in VOTE")
                }
                AppTextField {
                    id: incentiveField
                    inputMethodHints: Qt.ImhDigitsOnly
                    validator: DoubleValidator { bottom: 0; decimals: 4 }
                }
            }
        }
    }
}

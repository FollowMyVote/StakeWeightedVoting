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

    property VotingSystem votingSystem
    property var contestCreator

    Connections {
        target: contestCreator
        property var lastErrorTime
        onError: {
            // Ignore errors that come within 1 second of the last error
            console.log(lastErrorTime)
            if (new Date().getTime() - lastErrorTime < 1000)
                return

            var message = errorString.split(';')
            if (message.length > 1)
                message = message[1]
            else
                message = errorString

            NativeDialog.confirm("Error purchasing contest", message, function(){}, false)
            lastErrorTime = new Date().getTime()
        }
    }

    SwipeView {
        id: swiper
        anchors.fill: parent

        BasicContestForm {
            id: basicForm
            contestLimits: contestCreator.contestLimits
            coinsModel: votingSystem.coins
            onCompleted: swiper.currentIndex++
        }
        SponsorshipForm {
            onSponsorshipEnabledChanged: purchaseRequest.sponsorshipEnabled = sponsorshipEnabled
            onCompleted: {
                try {
                    var purchaseRequest = contestCreator.getPurchaseContestRequest()
                    purchaseRequest.contestType = ContestType.OneOfN
                    purchaseRequest.tallyAlgorithm = TallyAlgorithm.Plurality
                    purchaseRequest.name = basicForm.contestName
                    purchaseRequest.description = basicForm.contestDescription
                    for (var i = 0; i < basicForm.contestantModel.count; i++)
                        purchaseRequest.contestants.append(basicForm.contestantModel.get(i))
                    purchaseRequest.expiration = basicForm.contestExpiration
                    purchaseRequest.weightCoin = basicForm.weightCoinId
                    if (purchaseRequest.sponsorshipEnabled) {
                        purchaseRequest.sponsorMaxVotes = maxVotes? maxVotes : 0
                        purchaseRequest.sponsorMaxRevotes = maxRevotes? maxRevotes : 0
                        purchaseRequest.sponsorIncentive = incentive? incentive * 10000 : 0
                        purchaseRequest.sponsorEndDate = endTime
                    }

                    var dialog = purchaseDialog.createObject(createContestPage,
                                                             {"purchaseApi": purchaseRequest.submit()})
                    dialog.accepted.connect(function() {
                        // TODO: Actually pay
                        dialog.purchaseApi.paymentSent(dialog.selectedPriceIndex)
                        dialog.close()
                        createContestPage.navigationStack.pop()
                        dialog.destroy()
                    })
                    dialog.canceled.connect(function() {
                        dialog.close()
                        dialog.destroy()
                    })

                    dialog.open()
                } catch (exception) {
                    NativeDialog.confirm(qsTr("Error creating contest"),
                                         qsTr("An error occurred when processing your request: %1").arg(exception),
                                         function(){}, false)
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
        id: purchaseDialog

        Dialog {
            property var purchaseApi
            property var selectedPriceIndex: priceList.currentIdx
            property var selectedPrice: priceList.currentKey

            contentWidth: window.width * .6
            contentHeight: window.height * .6

            function updatePrices(totals, adjustments) {
                console.log(JSON.stringify(totals))
                console.log(JSON.stringify(adjustments))
                priceList.model = totals
                priceList.currentIdx = 0
                adjustmentRepeater.model = adjustments
            }
            Component.onCompleted: purchaseApi.prices([]).then(updatePrices)

            MouseArea {
                anchors.fill: parent
                onClicked: mouse.accepted = true
            }
            Column {
                anchors.fill: parent
                spacing: window.dp(8)

                Row {
                    spacing: window.dp(8)
                    AppText {
                        text: qsTr('Promo code')
                    }
                    AppTextField {
                        id: promoCodeField
                        onAccepted: promoCodeApplyButton.clicked()
                    }
                    AppButton {
                        id: promoCodeApplyButton
                        text: qsTr("Apply")
                        onClicked: purchaseApi.prices([promoCodeField.text]).then(updatePrices)
                    }
                }
                Row {
                    spacing: window.dp(8)
                    AppText {
                        text: qsTr("Pay with")
                    }
                    ComboList {
                        id: priceList
                        delegate: ComboListDelegateForSimpleVar {
                            property var coin: votingSystem.getCoin(modelData.coinId)
                            value: modelData.amount / Math.pow(10, coin.precision) + " " + coin.name
                        }
                    }
                }
                AppText {
                    text: qsTr("Price adjustments:")
                }
                Repeater {
                    id: adjustmentRepeater
                    delegate: AppText {
                        text: modelData.reason + ": " + (modelData.amount / 10000) + " VOTE"
                    }
                }
            }
        }
    }
}

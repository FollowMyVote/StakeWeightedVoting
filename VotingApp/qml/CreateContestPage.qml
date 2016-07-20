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
    property var dialog

    function showError(message) {
        var errorString = message
        // Ignore errors that come within 1 second of the last error
        if (new Date().getTime() - internal.lastErrorTime < 1000)
            return

        message = message.split(';')
        if (message.length > 1)
            message = message[1]
        else
            message = errorString

        if (dialog) {
            dialog.close()
            dialog.destroy()
            dialog = null
        }
        NativeDialog.confirm("Error purchasing contest", message, function(){}, false)
        internal.lastErrorTime = new Date().getTime()
    }

    QtObject {
        id: internal
        property var lastErrorTime
    }
    Connections {
        target: contestCreator
        onError: showError(errorString)
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
            onBack: swiper.currentIndex--

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
                    purchaseRequest.sponsorshipEnabled = sponsorshipEnabled
                    if (purchaseRequest.sponsorshipEnabled) {
                        purchaseRequest.sponsorMaxVotes = maxVotes? maxVotes : 0
                        purchaseRequest.sponsorMaxRevotes = maxRevotes? maxRevotes : 0
                        purchaseRequest.sponsorIncentive = incentive? incentive * 10000 : 0
                        purchaseRequest.sponsorEndDate = endTime
                    }

                    var purchaseApi = purchaseRequest.submit()
                    dialog = purchaseDialog.createObject(createContestPage, {"purchaseApi": purchaseApi})
                    dialog.accepted.connect(function() {
                        var transferPromise = votingSystem.chain.transfer(votingSystem.currentAccount.name,
                                                                          dialog.selectedPrice.payAddress,
                                                                          dialog.selectedPrice.amount,
                                                                          dialog.selectedPrice.coinId,
                                                                          dialog.selectedPrice.memo);
                        transferPromise.then(function() {
                            dialog.purchaseApi.paymentSent(dialog.selectedPriceIndex)
                            dialog.close()
                            createContestPage.navigationStack.pop()
                            dialog.destroy()
                            dialog = null
                        }, showError)
                    })
                    dialog.canceled.connect(function() {
                        dialog.close()
                        dialog.destroy()
                        dialog = null
                    })
                } catch (exception) {
                    NativeDialog.confirm(qsTr("Error creating contest"),
                                         qsTr("An error occurred when processing your request: %1").arg(exception),
                                         function(){}, false)
                    dialog.close()
                    dialog.destroy()
                    dialog = null
                }
            }
        }
    }
    PageControl {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        pages: swiper.count
        currentPage: swiper.currentIndex
        indicatorSize: window.dp(8)
    }

    Component {
        id: purchaseDialog

        Dialog {
            property var purchaseApi
            property var selectedPriceIndex: priceList.currentIdx
            property var selectedPrice: priceList.currentKey

            contentWidth: window.width * .8
            contentHeight: window.height * .6

            function updatePrices(totals, adjustments) {
                priceList.model = totals
                priceList.currentIdx = 0
                adjustmentRepeater.model = adjustments
            }
            Component.onCompleted: purchaseApi.prices([]).then(function(prices) {
                updatePrices(prices)
                open()
            })

            MouseArea {
                anchors.fill: parent
                onClicked: mouse.accepted = true
            }
            Column {
                anchors.fill: parent
                spacing: window.dp(8)

                RowLayout {
                    spacing: window.dp(8)
                    width: parent.width

                    AppText {
                        text: qsTr('Promo code')
                    }
                    AppTextField {
                        id: promoCodeField
                        onAccepted: promoCodeApplyButton.clicked()
                        Layout.fillWidth: true
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
                            property var coin: votingSystem.getCoin(modelData? modelData.coinId : 0)
                            value: (modelData? modelData.amount : 0) / Math.pow(10, coin.precision) + " " + coin.name
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

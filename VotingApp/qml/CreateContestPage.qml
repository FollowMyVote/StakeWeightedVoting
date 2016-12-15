import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import Qt.labs.settings 1.0
import QtQmlTricks.UiElements 2.0 as UI
import FollowMyVote.StakeWeightedVoting 1.0
import QuickPromise 1.0

Page {
    id: createContestPage
    header: ToolBar {
        ToolButton {
            contentItem: UI.SvgIconLoader {
                icon: "qrc:/icons/navigation/arrow_back.svg"
                color: Material.foreground
                size: height
            }
            onClicked: createContestCanceled()
        }
    }

    property VotingSystem votingSystem
    property var contestCreatorApi

    signal createContestCanceled
    signal contestFormComplete
    signal checkoutCanceled
    signal paymentAuthorizing
    signal paymentCanceled
    signal paymentSent
    signal paymentConfirmed
    signal confirmationFailed

    onCreateContestCanceled: StackView.view.pop()
    onContestFormComplete: beginCheckout()
    onPaymentConfirmed: {
        StackView.view.pop()
    }
    onConfirmationFailed: {
        StackView.view.pop()
    }

    function beginCheckout() {
        var purchaseRequest = contestCreatorApi.getPurchaseContestRequest()
        purchaseRequest.name = contestNameField.text
        purchaseRequest.description = contestDescriptionField.text
        purchaseRequest.weightCoin = votingSystem.coins.get(coinSelector.currentIndex).coinId

        for (var i = 0; i < contestantModel.count; ++i)
            purchaseRequest.contestants.append(contestantModel.get(i))

        checkoutDialogComponent.createObject(createContestPage, {purchaseApi: purchaseRequest.submit()}).open()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8

        TextField {
            id: contestNameField
            placeholderText: qsTr("Contest Name")
            Layout.fillWidth: true
            maximumLength: contestCreatorApi.contestLimits[ContestLimits.NameLength]
        }
        TextField {
            id: contestDescriptionField
            placeholderText: qsTr("Contest Description")
            Layout.fillWidth: true
            maximumLength: contestCreatorApi.contestLimits[ContestLimits.DescriptionHardLength]
        }
        Row {
            spacing: 4
            Label {
                text: qsTr("Coin:")
                anchors.baseline: coinSelector.baseline
            }
            ComboBox {
                id: coinSelector
                model: votingSystem.coins
                textRole: "name"
            }
        }
        ListView {
            Layout.fillHeight: true
            Layout.fillWidth: true
            model: ListModel {
                id: contestantModel
            }
            delegate: Row {
                Column {
                    Label {
                        text: model.name
                        font.bold: true
                        width: 300
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    }
                    Label {
                        text: model.description
                        width: 300
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    }
                }
                ToolButton {
                    contentItem: UI.SvgIconLoader {
                        icon: "qrc:/icons/action/delete_forever.svg"
                    }
                    onClicked: contestantModel.remove(index)
                }
            }
        }
        RowLayout {
            spacing: 4
            TextField {
                id: contestantNameField
                Layout.fillWidth: true
                placeholderText: qsTr("Contestant Name")
                maximumLength: contestCreatorApi.contestLimits[ContestLimits.ContestantNameLength]
            }
            TextField {
                id: contestantDescriptionField
                Layout.fillWidth: true
                placeholderText: qsTr("Contestant Description")
                maximumLength: contestCreatorApi.contestLimits[ContestLimits.ContestantDescriptionHardLength]
            }
            Button {
                text: qsTr("Add Contestant")
                onClicked: {
                    contestantModel.append({name: contestantNameField.text,
                                            description: contestantDescriptionField.text})
                    contestantNameField.text = ""
                    contestantDescriptionField.text = ""
                }
            }
        }
        RowLayout {
            spacing: 4
            Item { height: 1; Layout.fillWidth: true }
            Button {
                text: qsTr("Cancel")
                onClicked: createContestCanceled()
            }
            Button {
                text: qsTr("Checkout")
                onClicked: contestFormComplete()
            }
        }
    }

    Component {
        id: checkoutDialogComponent

        ShadowedPopup {
            id: checkoutDialog
            x: createContestPage.width / 2 - width / 2
            y: createContestPage.height / 2 - height / 2
            dim: true
            width: 600
            height: 400
            acceptButton.text: qsTr("Purchase")
            acceptButton.enabled: !waiting
            cancelButton.text: qsTr("Cancel Checkout")

            property var purchaseApi
            property bool waiting: false
            property bool paymentSent: false

            acceptButton.onClicked: {
                var total = priceSelector.model[priceSelector.currentIndex]
                var payPromise = walletConfirmationPopup.getAuthorization(function() {
                    return votingSystem.chain.transfer(votingSystem.currentAccount.name, total.payAddress,
                                                       total.amount, total.coinId, total.memo)
                })
                payPromise.then(function(trxid) {
                    createContestPage.paymentSent()
                    waiting = true
                    checkoutDialog.paymentSent = true
                    purchaseApi.paymentSent(priceSelector.currentIndex)
                }, function(error) {
                    //TODO: Show error?
                    console.log("Payment failed:", JSON.stringify(error))
                    paymentCanceled()
                })
            }
            cancelButton.onClicked: {
                checkoutCanceled()
                close()
            }
            onOpened: {
                purchaseApi.purchaseConfirmed.connect(function() {
                    createContestPage.paymentConfirmed()
                    close()
                })
                purchaseApi.purchaseFailed.connect(function() {
                    // TODO: show error in GUI
                    createContestPage.confirmationFailed()
                    console.log("Purchase failed")
                    close()
                })
                // Fetch prices
                purchaseApi.prices([]).then(setPrices, function(reason) {
                    // TODO: actually show the error in GUI
                    // The only reason this promise should fail is if the contest submission itself failed
                    console.log("Contest submission failed:", reason.split(';')[1])
                    close()
                })
            }

            function setPrices(totalsAndAdjustments) {
                priceSelector.model = totalsAndAdjustments.totals.map(function(total) {
                    var coin = votingSystem.getCoin(total.coinId)
                    total.text = (total.amount / Math.pow(10, coin.precision)).toString() + " " + coin.name
                    return total
                })
                adjustmentsRepeater.model = totalsAndAdjustments.adjustments.map(function(adjustment) {
                    var coin = votingSystem.getCoin("VOTE")
                    adjustment.text = adjustment.reason + ": " +
                            (adjustment.amount / Math.pow(10, coin.precision)).toString() + " " + coin.name
                    return adjustment
                })
            }

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 4
                enabled: !checkoutDialog.waiting

                RowLayout {
                    Layout.preferredWidth: 450
                    spacing: 4

                    TextField {
                        id: promoCodeField
                        Layout.fillWidth: true
                        placeholderText: qsTr("Promo code")
                    }
                    Button {
                        text: qsTr("Apply")
                        onClicked: {
                            var codes = promoCodeField.text.split(',').map(function(s) { return s.trim() })
                            checkoutDialog.purchaseApi.prices(codes).then(checkoutDialog.setPrices)
                        }
                    }
                }
                Column {
                    visible: adjustmentsRepeater.count
                    Label {
                        text: qsTr("Fees and Credits:")
                    }
                    Repeater {
                        id: adjustmentsRepeater
                        delegate: Label {
                            text: modelData.text
                        }
                    }
                }
                Row {
                    spacing: 4
                    Label {
                        text: qsTr("Price:")
                        anchors.baseline: priceSelector.baseline
                    }
                    ComboBox {
                        id: priceSelector
                        enabled: !!model && model.length > 1
                        textRole: "text"

                        Label {
                            anchors.centerIn: priceSelector
                            text: "Loading..."
                            visible: !priceSelector.model || priceSelector.model.length < 1
                        }
                    }
                }
            }

            Rectangle {
                anchors.fill: parent
                visible: checkoutDialog.waiting
                color: Qt.rgba(255,255,255,127)

                BusyIndicator {
                    id: waitingForWalletIndicator
                    anchors.centerIn: parent
                }
                Label {
                    anchors.top: waitingForWalletIndicator.bottom
                    UI.ExtraAnchors.horizontalFill: parent
                    anchors.margins: 8
                    text: checkoutDialog.paymentSent?
                              qsTr("Payment sent. Your contest is being created...")
                            : qsTr("Please confirm payment, including any applicable transaction fees, in your wallet.")
                    wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                    horizontalAlignment: Label.AlignHCenter
                }
            }
        }
    }
}

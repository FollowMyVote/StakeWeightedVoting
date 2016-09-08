import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import Qt.labs.settings 1.0
import QtQmlTricks.UiElements 2.0 as UI
import FollowMyVote.StakeWeightedVoting 1.0
import QuickPromise 1.0

ShadowedPopup {
    id: walletConfirmationPopup
    showButtons: false
    closePolicy: Popup.NoAutoClose
    modal: true
    dim: true
    height: confirmationRow.implicitHeight + 64
    width: confirmationRow.implicitWidth + 64
    x: window.width / 2 - width / 2
    y: window.height / 2 - height / 2
    z: 4
    
    // Show the popup until the transaction is either confirmed or canceled authorizeTransaction is a function
    // which submits the transaction to the wallet and returns a promise for its completion. This function will be
    // called once when the popup opens, and will be called again if the connection to the wallet fails during
    // authorization, after the wallet is reconnected.
    //
    // This function returns a promise which resolves or fails with the result of the final invocation of
    // authorizeTransaction().
    function getAuthorization(authorizeTransaction) {
        var myPromise = Q.promise()
        var makeTransactionPromise = function() {
            var promise = {ignored: false, promise: authorizeTransaction()}
            
            promise.promise = promise.promise.then(function(result) {
                if (!promise.ignored)
                    myPromise.resolve(result)
            }, function(reason) {
                if (!promise.ignored)
                    myPromise.reject(reason)
            })
            return promise
        }
        
        var transactionPromise = makeTransactionPromise()
        var handleReconnection = function() {
            transactionPromise = makeTransactionPromise()
        }
        var handleDisconnection = function() {
            transactionPromise.ignored = true
        }
        
        _votingSystem.blockchainWalletConnected.connect(handleReconnection)
        _votingSystem.blockchainWalletDisconnected.connect(handleDisconnection)
        
        walletConfirmationPopup.open()
        
        return myPromise.then(function(result) {
            _votingSystem.blockchainWalletConnected.disconnect(handleReconnection)
            _votingSystem.blockchainWalletDisconnected.disconnect(handleDisconnection)
            walletConfirmationPopup.close()
            return Q.resolved(result)
        }, function(reason) {
            _votingSystem.blockchainWalletConnected.disconnect(handleReconnection)
            _votingSystem.blockchainWalletDisconnected.disconnect(handleDisconnection)
            walletConfirmationPopup.close()
            return Q.rejected(reason)
        })
    }
    
    RowLayout {
        id: confirmationRow
        anchors.verticalCenter: parent.verticalCenter
        x: 16
        
        UI.SvgIconLoader {
            icon: "qrc:/icons/alert/warning.svg"
            color: Material.color(Material.Grey)
            size: 48
            
            SequentialAnimation on rotation {
                loops: Animation.Infinite
                PauseAnimation {
                    duration: 3000
                }
                NumberAnimation {
                    from: 0; to: 20
                    duration: 50
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    from: 20; to: -20
                    duration: 100
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    from: -20; to: 20
                    duration: 100
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    from: 20; to: -20
                    duration: 100
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    from: -20; to: 0
                    duration: 50
                    easing.type: Easing.InOutQuad
                }
            }
        }
        Label {
            font.pointSize: 12
            text: qsTr("Please authorize transaction in wallet")
        }
    }
}

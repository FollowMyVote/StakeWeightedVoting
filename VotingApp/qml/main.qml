import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import Qt.labs.settings 1.0
import QtQmlTricks.UiElements 2.0 as UI
import FollowMyVote.StakeWeightedVoting 1.0
import QuickPromise 1.0

ApplicationWindow {
    id: window
    visible: true

    ConnectionProgressPopup {
        id: connectionProgressPopup
        visible: true
    }
    WalletConfirmationPopup {
        id: walletConfirmationPopup
    }

    NavigationDrawer {
        id: _navigationDrawer
        height: window.height
        width: 300
        votingSystem: _votingSystem

        onCreateContestOpened: {
            mainStack.push(Qt.resolvedUrl("CreateContestPage.qml"),
                           {votingSystem: _votingSystem, contestCreatorApi: _votingSystem.backend.contestCreator})
            close()
        }
    }
    StackView {
        id: mainStack
        anchors.fill: parent

        initialItem: FeedPage {
            id: _feedPage
            navigationDrawer: _navigationDrawer
            votingSystem: _votingSystem

            onContestOpened: {
                mainStack.push(Qt.resolvedUrl("ContestDetailPage.qml"),
                               {contest: contest, votingSystem: _votingSystem})
            }
        }
    }

    Settings {
        property alias x: window.x
        property alias y: window.y
        property alias width: window.width
        property alias height: window.height
    }
    VotingSystem {
        id: _votingSystem

        function connectToBackendHelper() {
            if (currentAccount) {
                console.log("*** Connecting to backend as", currentAccount.name)
                connectionProgressPopup.text = qsTr("Connecting to Follow My Vote")
                connectToBackend("localhost", 17073, currentAccount.name)
            } else {
                console.log("*** Current account unset; No accounts in wallet?")
                connectionProgressPopup.text = qsTr("Wallet contains no accounts.\nCannot continue.")
                connectionProgressPopup.progress = .01
            }
        }

        // Normal startup routine:
        Component.onCompleted: {
            connectionProgressPopup.text = qsTr("Waiting for Bitshares wallet")
            connectionProgressPopup.progress = .3
            connectToBlockchainWallet(false)
        }
        onBlockchainWalletConnected: {
            connectionProgressPopup.progress = .6
            console.log("*** Blockchain connected")
            if (!backendIsConnected)
                // Unlock the wallet now, as we'll need unlocked it to connect to the backend
                _votingSystem.chain.unlockWallet()
            if (!currentAccount) {
                console.log("*** Syncing with blockchain")
                connectionProgressPopup.text = qsTr("Syncing with blockchain")
                syncWithBlockchain()
            } else if (!backendIsConnected) {
                connectToBackendHelper()
            } else {
                connectionProgressPopup.progress = 1
                connectionProgressPopup.text = "Finishing up"
                connectionProgressPopup.visible = false
            }
        }
        onBlockchainSynced: {
            connectionProgressPopup.progress = .9
            console.log("*** Blockchain synced")
            if (!backendIsConnected) {
                connectToBackendHelper()
            }
        }
        onBackendConnected: {
            connectionProgressPopup.progress = 1
            connectionProgressPopup.text = qsTr("Finishing up")
            connectionProgressPopup.visible = false
            console.log("*** Backend connected")
            _feedPage.contestListView.populateContests()
        }

        // Failure recovery:
        onBlockchainWalletDisconnected: {
            console.log("*** Blockchain disconnected; attempting to reconnect")
            connectionProgressPopup.progress = .45
            connectionProgressPopup.text = qsTr("Reconnecting to Bitshares wallet")
            connectToBlockchainWallet(false)
        }
        onBackendDisconnected: {
            console.log("*** Backend disconnected; attempting to reconnect")
            connectionProgressPopup.progress = .45
            connectionProgressPopup.text = qsTr("Reconnecting to Follow My Vote")
            connectToBackend("localhost", 17073, currentAccount.name)
        }
    }
}

import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import Qt.labs.settings 1.0
import QtQmlTricks.UiElements 2.0 as UI
import FollowMyVote.StakeWeightedVoting 1.0
import QtQml.StateMachine 1.0
import QuickPromise 1.0

ApplicationWindow {
    id: window
    visible: true

    property alias votingSystem: _votingSystem

    ConnectionProgressPopup {
        visible: !connectedState.active
    }

    Drawer {
        id: navigationDrawer
        width: 300
        height: window.height
    }
    StackView {
        id: mainStack
        anchors.fill: parent

        initialItem: Page {
            header: ToolBar {
                ToolButton {
                    contentItem: UI.SvgIconLoader {
                        id: img
                        icon: "qrc:/icons/navigation/menu.svg"
                    }
                    onClicked: navigationDrawer.open()
                }
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

        signal walletConnected
        signal synced
        signal connectedToBackend
    }
    StateMachine {
        id: globalStateMachine
        running: true
        initialState: connectingState
        childMode: State.ParallelStates

        State {
            id: connectingState
            initialState: waitingForWalletState

            State {
                id: waitingForWalletState

                onEntered: votingSystem.configureChainAdaptor(false).then(votingSystem.walletConnected)
                SignalTransition {
                    targetState: walletInitializingState
                    signal: votingSystem.walletConnected
                }
            }
            State {
                id: walletInitializingState

                onEntered: votingSystem.initialize().then(votingSystem.synced)
                SignalTransition {
                    guard: votingSystem.backendConnected
                    targetState: connectedState
                    signal: votingSystem.synced
                }
                SignalTransition {
                    guard: !votingSystem.backendConnected
                    targetState: backendConnectingState
                    signal: votingSystem.synced
                }
            }
            State {
                id: backendConnectingState

                onEntered: votingSystem.connectToBackend("localhost", 17073, votingSystem.currentAccount.name).then(
                              votingSystem.connectedToBackend)
                SignalTransition {
                    id: backendConnectingTransition
                    targetState: connectedState
                    signal: votingSystem.connectedToBackend
                }
            }
            State {
                id: connectedState
            }
        }
        StateMachine {
            id: appStateMachine
            initialState: feedState

            State {
                id: feedState
            }
        }
    }
}

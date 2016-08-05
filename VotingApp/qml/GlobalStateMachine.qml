import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.1
import QtQml.StateMachine 1.0
import QtGraphicalEffects 1.0
import Qt.labs.settings 1.0
import QtQmlTricks.UiElements 2.0 as UI
import FollowMyVote.StakeWeightedVoting 1.0
import QuickPromise 1.0

StateMachine {
    id: globalStateMachine
    childMode: State.ParallelStates

    property Page feedPage
    property VotingSystem votingSystem

    property alias connectionStateMachine: connectionStateMachine
    property alias appStateMachine: appStateMachine

    State {
        id: connectionStateMachine
        initialState: waitingForWalletState

        property alias waitingForWalletState: waitingForWalletState
        property alias blockchainSyncingState: blockchainSyncingState
        property alias backendConnectingState: backendConnectingState
        property alias connectedState: connectedState

        State {
            id: waitingForWalletState

            onEntered: votingSystem.configureChainAdaptor(false)
            SignalTransition {
                targetState: blockchainSyncingState
                signal: votingSystem.blockchainWalletConnected
            }
        }
        State {
            id: blockchainSyncingState

            onEntered: votingSystem.syncWithBlockchain()
            SignalTransition {
                guard: votingSystem.backendIsConnected
                targetState: connectedState
                signal: votingSystem.blockchainSynced
            }
            SignalTransition {
                guard: !votingSystem.backendIsConnected
                targetState: backendConnectingState
                signal: votingSystem.blockchainSynced
            }
            SignalTransition {
                targetState: waitingForWalletState
                signal: votingSystem.blockchainWalletDisconnected
            }
        }
        State {
            id: backendConnectingState

            onEntered: votingSystem.connectToBackend("localhost", 17073, votingSystem.currentAccount.name)
            SignalTransition {
                id: backendConnectingTransition
                targetState: connectedState
                signal: votingSystem.backendConnected
            }
            SignalTransition {
                targetState: waitingForWalletState
                signal: votingSystem.blockchainWalletDisconnected
            }
        }
        State {
            id: connectedState
            onEntered: {
                console.log("Voting system connected")
                if (feedPageState.active)
                    feedPage.contestListView.populateContests()
            }

            SignalTransition {
                targetState: waitingForWalletState
                signal: votingSystem.blockchainWalletDisconnected
            }
            SignalTransition {
                targetState: backendConnectingState
                signal: votingSystem.backendDisconnected
            }
        }
    }
    StateMachine {
        id: appStateMachine
        initialState: feedPageState

        property alias feedPageState: feedPageState

        State {
            id: feedPageState
            initialState: feedPopulatingState

            property alias feedPopulatingState: feedPopulatingState
            property alias feedNormalState: feedNormalState
            property alias feedPreloadingContestsState: feedPreloadingContestsState
            property alias feedOutOfContestsState: feedOutOfContestsState

            SignalTransition {
                signal: votingSystem.currentAccountChanged
                targetState: feedPopulatingState
            }

            State {
                id: feedPopulatingState
                onEntered: feedPage.contestListView.repopulateContests()

                SignalTransition {
                    signal: feedPage.populated
                    targetState: feedNormalState
                }
                SignalTransition {
                    signal: feedPage.outOfContests
                    targetState: feedOutOfContestsState
                }
            }
            State {
                id: feedNormalState

                SignalTransition {
                    signal: feedPage.needMoreContests
                    targetState: feedPreloadingContestsState
                }
                SignalTransition {
                    signal: feedPage.outOfContests
                    targetState: feedOutOfContestsState
                }
            }
            State {
                id: feedPreloadingContestsState

                SignalTransition {
                    signal: feedPage.populated
                    targetState: feedNormalState
                }
            }
            State {
                id: feedOutOfContestsState
                onEntered: console.log("No more contests in feed")
            }
        }
    }
}

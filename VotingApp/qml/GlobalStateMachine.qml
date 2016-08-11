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

    property FeedPage feedPage
    property ContestDetailPage contestDetailPage
    property CreateContestPage createContestPage
    property NavigationDrawer navigationDrawer
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

            onEntered: {
                if (!votingSystem.backendIsConnected)
                    // We know we'll be prompting the user to unlock the wallet in a moment, so go ahead and open the
                    // prompt now
                    votingSystem.chain.unlockWallet()
                votingSystem.syncWithBlockchain()
            }
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
        onEntered: console.log("Entered feedpage")
        onExited: console.log("Exited feedpage")

        property alias feedPageState: feedPageState
        property alias contestDetailPageState: contestDetailPageState
        property alias createContestPageState: createContestPageState

        State {
            id: feedPageState
            initialState: feedPopulatingState

            property alias feedPopulatingState: feedPopulatingState
            property alias feedNormalState: feedNormalState
            property alias feedPreloadingContestsState: feedPreloadingContestsState
            property alias feedOutOfContestsState: feedOutOfContestsState
            property alias feedPageHistoryState: feedPageHistoryState

            SignalTransition {
                signal: votingSystem.currentAccountChanged
                targetState: feedPopulatingState
            }
            SignalTransition {
                signal: feedPage.contestOpened
                targetState: contestDetailPageState
            }
            SignalTransition {
                signal: navigationDrawer.createContestOpened
                targetState: createContestPageState
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
                onEntered: console.log("Entered fns")
                onExited: console.log("Exited fns")

                SignalTransition {
                    signal: feedPage.needMoreContests
                    targetState: feedPreloadingContestsState
                }
                SignalTransition {
                    signal: feedPage.outOfContests
                    targetState: feedOutOfContestsState
                }
                SignalTransition {
                    signal: feedPage.repopulating
                    targetState: feedPopulatingState
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
                onExited: console.log("Exited foocs")

                SignalTransition {
                    signal: feedPage.repopulating
                    targetState: feedPopulatingState
                }
            }
        }
            HistoryState {
                id: feedPageHistoryState
                historyType: HistoryState.DeepHistory
            }
        State {
            id: contestDetailPageState
            initialState: contestDetailLoadingState

            property alias contestDetailLoadingState: contestDetailLoadingState
            property alias contestDetailNormalState: contestDetailNormalState

            State {
                id: contestDetailLoadingState

                SignalTransition {
                    signal: contestDetailPage.loaded
                    targetState: contestDetailNormalState
                    guard: !!contestDetailPage
                }
                SignalTransition {
                    signal: contestDetailPage.closed
                    targetState: feedPageHistoryState
                    guard: !!contestDetailPage
                }
            }
            State {
                id: contestDetailNormalState

                SignalTransition {
                    signal: contestDetailPage.closed
                    targetState: feedPageHistoryState
                    guard: !!contestDetailPage
                }
            }
        }
        State {
            id: createContestPageState
            initialState: fillContestFormState

            property alias fillContestFormState: fillContestFormState

            State {
                id: fillContestFormState

                SignalTransition {
                    signal: createContestPage.contestFormComplete
                    targetState: checkoutState
                }
                SignalTransition {
                    signal: createContestPage.createContestCanceled
                    targetState: feedPageHistoryState
                }
            }
            State {
                id: checkoutState

                SignalTransition {
                    signal: createContestPage.paymentAuthorizing
                    targetState: paymentAuthorizingState
                }
                SignalTransition {
                    signal: createContestPage.checkoutCanceled
                    targetState: fillContestFormState
                }
            }
            State {
                id: paymentAuthorizingState

                SignalTransition {
                    signal: createContestPage.paymentSent
                    targetState: waitingForConfirmationState
                }
                SignalTransition {
                    signal: createContestPage.paymentCanceled
                    targetState: checkoutState
                }
                SignalTransition {
                    signal: createContestPage.checkoutCanceled
                    targetState: fillContestFormState
                }
            }
            State {
                id: waitingForConfirmationState

                SignalTransition {
                    signal: createContestPage.paymentConfirmed
                    targetState: feedPopulatingState
                }
                SignalTransition {
                    signal: createContestPage.confirmationFailed
                    targetState: feedPageHistoryState
                }
                SignalTransition {
                    signal: createContestPage.checkoutCanceled
                    targetState: fillContestFormState
                }
            }
        }
    }
}

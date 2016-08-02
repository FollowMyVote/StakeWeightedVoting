import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import QtQml.StateMachine 1.0
import QtGraphicalEffects 1.0
import Qt.labs.settings 1.0
import QtQmlTricks.UiElements 2.0 as UI
import FollowMyVote.StakeWeightedVoting 1.0
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

            ListView {
                id: contestListView
                anchors.fill: parent
                model: ListModel {
                    id: contestListModel
                }
                delegate: Rectangle {
                    layer.enabled: true
                    layer.effect: DropShadow {
                        transparentBorder: true
                    }
                }

                property var generator

                signal populated
                signal outOfContests

                function populateContests() {
                    if (!generator) {
                        generator = votingSystem.backend.getFeedGenerator()
                        console.log("Setting generator", generator)
                    }

                    console.log("Populating contest list, %1/%2".arg(contentHeight).arg(height))
                    if (contentHeight < height) {
                        generator.getContest().then(function(contestDescription) {
                            if (!contestDescription)
                                throw "Out of contests"

                            return votingSystem.chain.getContest(contestDescription.contestId).then(function(contest) {
                                console.log("Got contest", JSON.stringify(contest))
                                contest.votingStake = contestDescription.votingStake
                                contest.tracksLiveResults = contestDescription.tracksLiveResults
                                return contest
                            })
                        }).then(contestListModel.append, function(error) {
                            console.log("Error fetching contest", JSON.stringify(error))
                        }).then(function(contest) {
                            contestListModel.append(contest)
                            contestListView.populateContests()
                        }, function(error) {
                            if (error === "Out of contests")
                                contestListView.outOfContests()
                        })
                    } else contestListView.populated()
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
    }
    StateMachine {
        id: globalStateMachine
        running: true
        childMode: State.ParallelStates

        State {
            initialState: waitingForWalletState

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
                    if (feedState.active)
                        contestListView.populateContests()
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
            initialState: feedState

            State {
                id: feedState
                initialState: feedPopulatingState

                State {
                    id: feedPopulatingState

                    SignalTransition {
                        signal: contestListView.populated
                        targetState: feedNormalState
                    }
                    SignalTransition {
                        signal: contestListView.outOfContests
                        targetState: feedOutOfContestsState
                    }
                }
                State {
                    id: feedNormalState

                    SignalTransition {
                        signal: contestListView.outOfContests
                        targetState: feedOutOfContestsState
                    }
                }
                State {
                    id: feedOutOfContestsState
                }
            }
        }
    }
}

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
                        color: Material.foreground
                    }
                    onClicked: navigationDrawer.open()
                }
            }

            ListView {
                id: contestListView
                anchors.fill: parent
                anchors.topMargin: 8
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                spacing: 8
                model: ListModel {
                    id: contestListModel
                }
                delegate: ContestDelegate {
                    layer {
                        enabled: true
                        effect: DropShadow {
                            transparentBorder: true
                        }
                    }
                    width: parent.width
                }
                footer: Item {
                    width: parent.width
                    height: 60

                    Label {
                        text: qsTr("No more contests")
                        anchors.centerIn: parent
                        visible: feedOutOfContestsState.active
                    }
                    Button {
                        // This button should never actually appear on screen, but in case I've got a bug in my contest
                        // preloading code, I put it here anyways
                        anchors.fill: parent
                        text: qsTr("Load more contests")
                        onClicked: contestListView.needMoreContests()
                        visible: feedNormalState.active
                    }
                    BusyIndicator {
                        anchors.centerIn: parent
                        running: connectedState.active &&
                                 (feedPopulatingState.active || feedPreloadingContestsState.active)
                    }

                }

                property var generator

                signal populated
                signal needMoreContests
                signal outOfContests

                onContentYChanged: {
                    if (!feedNormalState.active)
                        return
                    var verticalPixelsRemainingToScroll = (contentHeight - height) - contentY
                    if (verticalPixelsRemainingToScroll < height / 2)
                        needMoreContests()
                }
                onNeedMoreContests: {
                    generator.getContests(3).then(function(contestDescriptions) {
                        return Q.all(contestDescriptions.map(loadContestFromChain))
                    }).then(function(contests) {
                        contests.map(contestListModel.append)
                        populated()
                        if (contests.length < 3)
                            outOfContests()
                    })
                }

                function loadContestFromChain(contestDescription) {
                    return votingSystem.chain.getContest(contestDescription.contestId).then(function(contest) {
                        contest.votingStake = contestDescription.votingStake
                        contest.tracksLiveResults = contestDescription.tracksLiveResults
                        return contest
                    })
                }
                function fetchContest() {
                    return generator.getContest().then(loadContestFromChain).then(function(contest) {
                        contestListModel.append(contest)
                    }, function(error) {
                        contestListView.outOfContests()
                        // Reject the resulting promise
                        throw false
                    })
                }
                function populateContests() {
                    if (!generator) {
                        generator = votingSystem.backend.getFeedGenerator()
                        console.log("Setting generator", generator)
                    }

                    console.log("Populating contest list, %1/%2".arg(contentHeight).arg(height))
                    if (contentHeight < height)
                        fetchContest().then(contestListView.populateContests)
                    else
                        contestListView.populated()
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
                    SignalTransition {
                        signal: contestListView.needMoreContests
                        targetState: feedPreloadingContestsState
                    }
                }
                State {
                    id: feedPreloadingContestsState

                    SignalTransition {
                        signal: contestListView.populated
                        targetState: feedNormalState
                    }
                }
                State {
                    id: feedOutOfContestsState
                }
            }
        }
    }
}

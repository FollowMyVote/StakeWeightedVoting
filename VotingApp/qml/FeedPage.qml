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

Page {
    id: feedPage
    property NavigationDrawer navigationDrawer
    property VotingSystem votingSystem
    property State feedPageState
    property State connectionStateMachine

    property alias contestListView: contestListView

    signal populated
    signal repopulating
    signal needMoreContests
    signal outOfContests
    signal contestOpened(Contest contest)

    onNeedMoreContests: {
        contestListView.generator.getContests(3).then(function(contestDescriptions) {
            return Q.all(contestDescriptions.map(function(c) { return contestListView.loadContestFromChain(c) }))
        }).then(function(contests) {
            contests.map(function(c) { contestListModel.append(c) })
            populated()
            if (contests.length < 3)
                outOfContests()
        })
    }

    header: ToolBar {
        Row {
            ToolButton {
                contentItem: UI.SvgIconLoader {
                    icon: "qrc:/icons/navigation/menu.svg"
                    color: Material.foreground
                }
                onClicked: navigationDrawer.open()
            }
            ToolButton {
                contentItem: UI.SvgIconLoader {
                    icon: "qrc:/icons/navigation/refresh.svg"
                    color: Material.foreground
                }
                onClicked: repopulating()
            }
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
            id: contestDelegate
            layer {
                enabled: true
                effect: DropShadow {
                    transparentBorder: true
                }
            }
            width: parent.width
            contest: model.contest
            votingSystem: feedPage.votingSystem

            MouseArea {
                anchors.fill: parent
                z: -1
                onClicked: contestOpened(contestDelegate.contest)
            }
        }
        footer: Item {
            width: parent.width
            height: 60

            Label {
                text: qsTr("No more contests")
                anchors.centerIn: parent
                visible: feedPageState.feedOutOfContestsState.active
            }
            Button {
                // This button should never actually appear on screen, but in case I've got a bug in my contest
                // preloading code, I put it here anyways
                anchors.fill: parent
                text: qsTr("Load more contests")
                onClicked: needMoreContests()
                visible: feedPageState.feedNormalState.active
            }
            BusyIndicator {
                anchors.centerIn: parent
                running: connectionStateMachine.connectedState.active &&
                         (feedPageState.feedPopulatingState.active || feedPageState.feedPreloadingContestsState.active)
            }
        }

        property var generator

        onContentYChanged: {
            if (!feedPageState.feedNormalState.active)
                return
            var verticalPixelsRemainingToScroll = (contentHeight - height) - contentY
            if (verticalPixelsRemainingToScroll < height / 2)
                needMoreContests()
        }

        function loadContestFromChain(contestDescription) {
            console.log("Fetching data for", JSON.stringify(contestDescription))
            return votingSystem.chain.getContest(contestDescription.contestId).then(function(contest) {
                contest.votingStake = contestDescription.votingStake
                contest.tracksLiveResults = contestDescription.tracksLiveResults
                return {contest: contest}
            })
        }
        function fetchContest() {
            return generator.getContest().then(loadContestFromChain).then(function(contest) {
                console.log(JSON.stringify(contest))
                contestListModel.append(contest)
            }, function(error) {
                outOfContests()
                // Reject the resulting promise
                var promise = Q.promise()
                promise.reject()
                return promise
            })
        }
        function repopulateContests() {
            console.log("Repopulating contest feed")
            generator = null
            contestListModel.clear()
            // Delay this for a moment so the ListView has a chance to clear
            Q.setTimeout(populateContests, 20)
        }
        function populateContests() {
            if (!votingSystem.backend)
                return
            if (!generator) {
                generator = votingSystem.backend.getFeedGenerator()
                console.log("Setting generator", generator)
            }

            console.log("Populating contest list, %1/%2".arg(contentHeight).arg(height))
            if (contentHeight < height)
                fetchContest().then(contestListView.populateContests)
            else
                populated()
        }
    }
}

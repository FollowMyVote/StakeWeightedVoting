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
    id: feedPage
    property NavigationDrawer navigationDrawer
    property VotingSystem votingSystem

    property alias contestListView: contestListView

    signal populated
    signal repopulating
    signal outOfContests
    signal contestOpened(Contest contest)

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
                onClicked: contestListView.repopulateContests()
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
        delegate: Component {
            id: contestantComponent
            ContestDelegate {
                id: contestDelegate
                layer {
                    enabled: true
                    effect: DropShadow {
                        transparentBorder: true
                    }
                }
                UI.ExtraAnchors.horizontalFill: parent
                votingSystem: feedPage.votingSystem
                contest: {
                    if (model && model.contest && model.contest.name)
                        return model.contest
                    console.log("Something's fucked up.")
                    return contestListModel.get(index)
                }

                MouseArea {
                    anchors.fill: parent
                    z: -1
                    onClicked: contestOpened(contestDelegate.contest)
                }
            }
        }
        footer: Item {
            width: parent.width
            height: 60

            Label {
                text: qsTr("No more contests")
                anchors.centerIn: parent
                visible: !!contestListView.generator && contestListView.generator.isOutOfContests
            }
            Button {
                // This button should never actually appear on screen, but in case I've got a bug in my contest
                // preloading code, I put it here anyways
                anchors.fill: parent
                text: qsTr("Load more contests")
                onClicked: contestListView.fetchMoreContests()
                visible: !!contestListView.generator &&
                         !contestListView.generator.isOutOfContests && !contestListView.generator.isFetchingContests
            }
            BusyIndicator {
                anchors.centerIn: parent
                running: !!contestListView.generator && contestListView.generator.isFetchingContests
            }
        }

        property var generator

        property bool watchYChanges: true
        onContentYChanged: {
            if (!watchYChanges || !generator || generator.isFetchingContests || generator.isOutOfContests)
                return
            var verticalPixelsRemainingToScroll = (contentHeight - height) - contentY
            if (verticalPixelsRemainingToScroll < height / 2) {
                watchYChanges = false
                fetchMoreContests().then(function() { watchYChanges = true })
            }
        }

        function loadContestFromChain(contestDescription) {
            console.log("Fetching data for", JSON.stringify(contestDescription))
            return votingSystem.chain.getContest(contestDescription.contestId).then(function(contest) {
                // Work around a bug in Qt causing contest to be garbage collected while still in use
                generator.takeOwnership(contest)
                contest.votingStake = contestDescription.votingStake
                contest.tracksLiveResults = contestDescription.tracksLiveResults
                return {contest: contest}
            })
        }
        function fetchContest() {
            return generator.getContest().then(loadContestFromChain).then(function(contest) {
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
            repopulating()
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
        function fetchMoreContests() {
            if (!generator || generator.isFetchingContests || generator.isOutOfContests)
                return
            return generator.getContests(3).then(function(contestDescriptions) {
                return Q.all(contestDescriptions.map(function(c) { return contestListView.loadContestFromChain(c) }))
            }).then(function(contests) {
                contests.map(function(c) {
                    if (c && c.hasOwnProperty("contest") && !!c.contest)
                        contestListModel.append(c)
                    else
                        console.error("Got something, but it's not a contest:", JSON.stringify(c))
                })
                populated()
                if (contests.length < 3)
                    outOfContests()
            })
        }
    }
}

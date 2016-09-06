import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.1
import QtQml 2.2
import QtGraphicalEffects 1.0
import Qt.labs.settings 1.0
import QtQmlTricks.UiElements 2.0 as UI
import FollowMyVote.StakeWeightedVoting 1.0
import QuickPromise 1.0

Rectangle {
    id: delegate
    height: contestMainColumn.height + 8

    property var contest
    property VotingSystem votingSystem
    property int status: {
        // Mention these, so the binding re-evaluates if they change
        contest.pendingDecision.opinions && contest.pendingDecision.writeIns && contest.officialDecision &&
                contest.officialDecision.opinions && contest.officialDecision.writeIns
        console.log(JSON.stringify(contest.officialDecision))
        if (!contest || !contest.pendingDecision || !contest.officialDecision || contest.officialDecision.isNull())
            return contest.pendingDecision.isNull()? ContestDecisionStatus.NoDecision
                                                   : ContestDecisionStatus.PendingDecision
        return contest.officialDecision.isEqual(contest.pendingDecision)?
                    ContestDecisionStatus.OfficialDecision : ContestDecisionStatus.PendingDecision
    }

    Column {
        id: contestMainColumn
        UI.ExtraAnchors.topDock: parent
        anchors.margins: 4
        spacing: 8

        RowLayout {
            width: parent.width

            Column {
                id: contestHeaderLeftColumn

                Label {
                    id: contestCoinLabel
                    text: votingSystem.getCoin(contest.coin).name
                }
                Label {
                    id: contestStartTimeLabel
                    text: contest.startTime.toLocaleString(Qt.locale(), Locale.ShortFormat)
                }
            }
            Item { height: 1; Layout.fillWidth: true }
            Button {
                id: favoriteButton
                implicitHeight: contestHeaderLeftColumn.height
                implicitWidth: height
                Layout.preferredWidth: implicitWidth
                property bool favorited: false
                contentItem: UI.SvgIconLoader {
                    icon: favoriteButton.favorited? "qrc:/icons/action/favorite.svg"
                                                  : "qrc:/icons/action/favorite_border.svg"
                    color: Material.accent
                }
                background: Item {}
                onClicked: favorited = !favorited
            }
        }
        Label {
            UI.ExtraAnchors.horizontalFill: contestMainColumn
            text: contest.name
            font.weight: Font.Bold
            font.pixelSize: 20
            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
        }
        Label {
            UI.ExtraAnchors.horizontalFill: contestMainColumn
            text: contest.description
            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
        }
        UI.GridContainer {
            UI.ExtraAnchors.horizontalFill: contestMainColumn
            rowSpacing: 4
            colSpacing: 8
            cols: Math.min(Math.floor(width / 300), contest.contestants.length)

            Repeater {
                model: contest.contestants
                Button {
                    text: modelData.name + "\n" + modelData.description
                    highlighted: !!contest.pendingDecision.opinions[index.toString()]
                    onClicked: {
                        var opinions = {}
                        opinions[index.toString()] = !highlighted
                        contest.pendingDecision.opinions = opinions
                    }
                }
            }
        }
        RowLayout {
            width: parent.width
            spacing: 8

            UI.SvgIconLoader {
                id: icon
                icon: delegate.status === ContestDecisionStatus.OfficialDecision? "qrc:/icons/action/check_circle.svg"
                                                                                : "qrc:/icons/alert/warning.svg"
                color: Material.color(delegate.status === ContestDecisionStatus.OfficialDecision? Material.Green
                                                                                                : Material.Grey)
                Layout.fillHeight: true
                size: height
            }
            Label {
                text: {
                    if (delegate.status === ContestDecisionStatus.NoDecision)
                        return "Undecided"
                    if (delegate.status === ContestDecisionStatus.PendingDecision)
                        return "Pending"
                    if (delegate.status === ContestDecisionStatus.OfficialDecision)
                        return "Official"
                }
                font.pixelSize: icon.height / 2
                font.weight: Font.DemiBold
                color: delegate.status === ContestDecisionStatus.OfficialDecision? Material.foreground
                                                                                 : Material.color(Material.Grey)
            }

            Item { height: 1; Layout.fillWidth: true }
            Button {
                text: qsTr("Cancel")
                onClicked: votingSystem.cancelPendingDecision(contest)
            }
            Button {
                text: qsTr("Cast Vote")
                onClicked: votingSystem.castPendingDecision(contest)
            }
        }
    }
}

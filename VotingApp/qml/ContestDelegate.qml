import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import QtQml 2.2
import QtGraphicalEffects 1.0
import Qt.labs.settings 1.0
import QtQmlTricks.UiElements 2.0 as UI
import FollowMyVote.StakeWeightedVoting 1.0
import QuickPromise 1.0

Rectangle {
    id: delegate
    height: contestMainColumn.height + 8
    // Max width: 4.5 in (25.4 mm per in)
    width: Math.min(parent.width, Screen.pixelDensity * 25.4 * 4.5)
    anchors.horizontalCenter: parent.horizontalCenter

    property bool shouldElide: false
    property var contest
    property VotingSystem votingSystem
    property int status: {
        // Mention these, so the binding re-evaluates if they change
        contest.pendingDecision.opinions && contest.pendingDecision.writeIns && contest.officialDecision &&
                contest.officialDecision.opinions && contest.officialDecision.writeIns

        if (!contest || !contest.pendingDecision)
            return ContestDecisionStatus.NoDecision
        if (contest.pendingDecision.isNull() && (!contest.officialDecision || contest.officialDecision.isNull()))
            return ContestDecisionStatus.NoDecision
        if (contest.officialDecision && contest.pendingDecision.isEqual(contest.officialDecision))
            return ContestDecisionStatus.OfficialDecision
        return ContestDecisionStatus.PendingDecision
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
        Column {
            width: parent.width

            Label {
                id: descriptionText
                UI.ExtraAnchors.horizontalFill: parent
                text: contest.description
                wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                height: elided? font.pixelSize * 4.5 : implicitHeight
                clip: elided

                property bool elided: shouldElide && contentHeight > font.pixelSize * 11

                Rectangle {
                    visible: parent.elided
                    UI.ExtraAnchors.bottomDock: parent
                    height: parent.font.pixelSize * 1.5
                    gradient: Gradient {
                        GradientStop {
                            color: "transparent"
                            position: 0
                        }
                        GradientStop {
                            color: delegate.color
                            position: 1
                        }
                    }
                }
            }
            UI.SvgIconLoader {
                visible: descriptionText.elided
                icon: "qrc:/icons/navigation/more_horiz.svg"
                color: descriptionText.color
                size: descriptionText.font.pixelSize
                anchors.horizontalCenter: parent.horizontalCenter
            }
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
        ContestStatusRow {
            id: buttonRowContainer
            width: parent.width
            contest: contestDelegate.contest
            state: {
                if (contestDelegate.status === ContestDecisionStatus.NoDecision)
                    return "HIDDEN"
                if (contestDelegate.status === ContestDecisionStatus.PendingDecision)
                    return "SHOW_PENDING"
                if (contestDelegate.status === ContestDecisionStatus.OfficialDecision)
                    return "SHOW_OFFICIAL"
            }
        }
    }
}

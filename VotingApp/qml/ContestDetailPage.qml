import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.1
import QtCharts 2.0
import QtQml.StateMachine 1.0
import QtGraphicalEffects 1.0
import Qt.labs.settings 1.0
import QtQmlTricks.UiElements 2.0 as UI
import FollowMyVote.StakeWeightedVoting 1.0
import QuickPromise 1.0

Page {
    id: contestDetailPage

    property alias contest: contestDelegate.contest
    property VotingSystem votingSystem

    property var resultsApi: votingSystem.backend.getContestResults(contest)
    Binding {
        target: resultsApi
        property: "results"
        value: resultSeries
    }

    signal loaded
    signal closed

    header: ToolBar {
        ToolButton {
            contentItem: UI.SvgIconLoader {
                icon: "qrc:/icons/navigation/arrow_back.svg"
                color: Material.foreground
            }
            onClicked: contestDetailPage.StackView.view.pop()
        }
    }

    Flickable {
        anchors.fill: parent

        Column {
            UI.ExtraAnchors.horizontalFill: parent
            anchors.margins: 4
            spacing: 4

            ContestDelegate {
                id: contestDelegate
                width: parent.width
                votingSystem: contestDetailPage.votingSystem
            }
            ChartView {
                UI.ExtraAnchors.horizontalFill: parent
                anchors.top: contestDelegate.bottom
                height: 400

                BarSeries {
                    id: resultSeries
                }
            }
        }
    }
}

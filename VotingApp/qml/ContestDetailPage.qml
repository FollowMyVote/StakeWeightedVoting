import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.1
import QtCharts 2.0
import QtGraphicalEffects 1.0
import Qt.labs.settings 1.0
import QtQmlTricks.UiElements 2.0 as UI
import FollowMyVote.StakeWeightedVoting 1.0
import QuickPromise 1.0

Page {
    id: contestDetailPage

    property alias contest: contestDelegate.contest
    property VotingSystem votingSystem
    property var resultMap: {
        var contestantResults = contest.resultsApi.contestantResults
        var precision = Math.pow(10, votingSystem.getCoin(contest.coin).precision)
        var contestantNameToTally = contest.contestants.reduce(function(results, contestant, contestantIndex) {
            var tally = (contestantResults.length > contestantIndex)? contestantResults[contestantIndex] : 0
            results[contestant.name] = tally / precision
            return results
        }, {})
        for (var writeInName in contest.resultsApi.writeInResults)
            contestantNameToTally[writeInName] = contest.resultsApi.writeInResults[writeInName] / precision
        return contestantNameToTally
    }

    signal loaded
    signal closed
    onClosed: {
        contestDetailPage.StackView.view.pop()
    }

    header: ToolBar {
        ToolButton {
            contentItem: UI.SvgIconLoader {
                icon: "qrc:/icons/navigation/arrow_back.svg"
                color: Material.foreground
            }
            onClicked: contestDetailPage.closed()
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
                id: resultsChart
                UI.ExtraAnchors.horizontalFill: parent
                height: 400
                legend.visible: false
                ToolTip.delay: 300

                BarSeries {
                    id: resultSeries
                    axisX: BarCategoryAxis { categories: Object.keys(resultMap) }
                    axisY: ValueAxis {
                        min: 0
                        max: !!resultMap && !!resultMap.reduce? resultMap.reduce(function(max, tally) {
                            return Math.max(max, tally)
                        }, 0) : 100
                        onRangeChanged: applyNiceNumbers()
                    }
                    BarSet { values: Object.keys(resultMap).map(function(name) { return resultMap[name] }) }

                    onHovered: {
                        if (status) {
                            var candidate = axisX.categories[index]
                            var message = qsTr("%1 has received %2 votes").arg(candidate)
                                                                          .arg(resultMap[candidate].toString())
                            resultsChart.ToolTip.show(message, 5000)
                        } else
                            resultsChart.ToolTip.hide()
                    }
                }
            }
        }
    }
}

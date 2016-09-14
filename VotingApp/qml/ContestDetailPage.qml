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
    property var candidates: resultMap? Object.keys(resultMap) : []
    property var tallies: candidates.map(function(name) { return resultMap[name] })

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
                size: height
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
                localizeNumbers: true
                ToolTip.delay: 300

                BarSeries {
                    id: resultSeries
                    axisX: BarCategoryAxis { categories: candidates }
                    axisY: ValueAxis {
                        min: 0
                        max: tallies? Math.max.apply(null, tallies) : 100
                        onRangeChanged: applyNiceNumbers()
                    }
                    BarSet { values: tallies }

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
            Repeater {
                id: decisionRecordsRepeater
                model: ListModel {
                    id: decisionGroupsList
                }
                delegate: Label {
                    text: ((records.count === 1)? "A vote" : (records.count + " votes")) + " from " + voter
                }

                property var generator

                Component.onCompleted: {
                    generator = contest.resultsApi.getDecisionGenerator()
                    generator.getDecisions(100).then(function(decisionInfoList) {
                        // Return a promise for a list of DecisionRecords by joining a list of promises thereof
                        return Q.all(decisionInfoList.map(function(info) {
                            return votingSystem.chain.getDecisionRecord(info.decisionId).then(function(record) {
                                console.log(record.timestamp)
                                // Add the counted property to the DecisionRecord
                                record.counted = info.counted
                                return record
                            })
                        })).then(function(records) {
                            // Coalesce records into groups by voter and append groups to list model
                            records.reduce(function(groups, record) {
                                if (groups.length === 0 || groups[groups.length-1].voter !== record.voter)
                                    groups.push({voter: record.voter, records: []})
                                groups[groups.length-1].records.push(record)
                                return groups
                            }, []).map(function(group) {
                                decisionGroupsList.append(group)
                            })
                        })
                    })
                }
            }
        }
    }
}

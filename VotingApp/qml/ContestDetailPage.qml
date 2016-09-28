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
    property var coin: votingSystem.getCoin(contest.coin)
    property VotingSystem votingSystem
    property var resultMap: {
        var contestantResults = contest.resultsApi.contestantResults
        var precision = Math.pow(10, coin.precision)
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
        flickableDirection: Flickable.VerticalFlick
        contentHeight: contestDetailColumn.height

        Column {
            id: contestDetailColumn
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
                model: []
                delegate: Rectangle {
                    height: recordGroupColumn.height + 16
                    width: parent.width
                    layer {
                        enabled: true
                        effect: DropShadow {
                            transparentBorder: true
                        }
                    }

                    Column {
                        id: recordGroupColumn
                        UI.ExtraAnchors.topDock: parent
                        anchors.margins: 8

                        property var currentDecision: modelData.records[modelData.records.length - 1]
                        property var replacedDecisions: modelData.records.slice(0, modelData.records.length - 1)

                        Label {
                            property var weight: recordGroupColumn.currentDecision.weight
                            text: qsTr("Voter: %1\nWeight: %2").arg(recordGroupColumn.currentDecision.voter)
                                                               .arg(contestDetailPage.coin.formatAmount(weight))
                        }
                        Column {
                            id: oldDecisionsColumn
                            width: parent.width
                            visible: recordGroupColumn.replacedDecisions.length

                            property bool expanded: false

                            Item {
                                UI.ExtraAnchors.horizontalFill: parent
                                height: showOldDecisionsRow.height
                                Row {
                                    id: showOldDecisionsRow
                                    UI.SvgIconLoader {
                                        id: expandIcon
                                        icon: "qrc:/icons/navigation/expand_more.svg"
                                        rotation: oldDecisionsColumn.expanded? 180 : 0
                                        size: showOldDecisionsLabel.height
                                        anchors.verticalCenter: showOldDecisionsLabel.verticalCenter
                                        Behavior on rotation { RotationAnimation {} }
                                    }
                                    Label {
                                        id: showOldDecisionsLabel
                                        property int count: recordGroupColumn.replacedDecisions.length
                                        text: count === 1? qsTr("Show one replaced decision")
                                                         : qsTr("Show %1 replaced decisions").arg(count)
                                    }
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: oldDecisionsColumn.expanded = !oldDecisionsColumn.expanded
                                }
                            }
                            Column {
                                height: oldDecisionsColumn.expanded? implicitHeight : 0
                                clip: true
                                UI.ExtraAnchors.horizontalFill: parent
                                anchors.leftMargin: 8
                                Behavior on height { NumberAnimation {} }

                                Repeater {
                                    model: recordGroupColumn.replacedDecisions
                                    delegate: DecisionDelegate {
                                        contest: contestDetailPage.contest
                                        decision: modelData
                                        UI.ExtraAnchors.horizontalFill: parent
                                    }
                                }
                            }
                        }
                        DecisionDelegate {
                            UI.ExtraAnchors.horizontalFill: parent
                            contest: contestDetailPage.contest
                            decision: recordGroupColumn.currentDecision
                        }
                    }
                }

                // We store decisions here until we have their entire group
                property var pendingDecisions: []
                property var finalDecisionRecords: []
                function showDecisionGroup(group) {
                    if (group.voter === votingSystem.currentAccount.name)
                        finalDecisionRecords.unshift(group)
                    else
                        finalDecisionRecords.push(group)
                    decisionRecordsRepeater.model = finalDecisionRecords
                }
                function fetchAllDecisions(generator) {
                    generator.getDecisions(100).then(function(decisionInfos) {
                        return Q.all(decisionInfos.map(function(info) {
                            return votingSystem.chain.getDecisionRecord(info.decisionId).then(function(record) {
                                // Add the counted property to the DecisionRecord
                                record.counted = info.counted
                                return record
                            })
                        }))
                    }).then(function(decisionRecords) {
                        // Iterate over the new decisions, moving groups to pendingGroups as we find them
                        for (var i = 0; i < decisionRecords.length; ++i) {
                            // First, append the new decision to pendingDecisions
                            var decision = decisionRecords[i]
                            pendingDecisions.push(decision)

                            // A counted decision marks the end of a group
                            if (decision.counted) {
                                // pendingDecisions contains a full group. Display it
                                showDecisionGroup({voter: decision.voter, records: pendingDecisions})
                                pendingDecisions = []
                            }
                        }

                        if (generator.isOutOfDecisions) {
                            console.log("Done fetching decisions.")
                            if (pendingDecisions.length)
                                console.error("Pending decisions left over:", JSON.stringify(pendingDecisions))
                            // Clean up
                            pendingDecisions = []
                            finalDecisionRecords = []
                            return
                        }
                        // Still more work to do; keep going
                        fetchAllDecisions(generator)
                    })
                }
                function refreshDecisions() {
                    fetchAllDecisions(contest.resultsApi.getDecisionGenerator())
                }

                Component.onCompleted: {
                    refreshDecisions()
                    contest.resultsApi.resultsChanged.connect(refreshDecisions)
                }
            }
        }
    }
}

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0

import Material 0.1

import jbQuick.Charts 1.0

import FollowMyVote.StakeWeightedVoting 1.0

Page {
    title: qsTr("Contest Results")

    property alias contest: delegate.displayContest

    Flickable {
        anchors.fill: parent
        interactive: true
        contentWidth: width
        contentHeight: contestColumn.height

        Column {
            id: contestColumn
            width: parent.width

            ContestDelegate {
                id: delegate
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Units.dp(8)
                }
            }
            Chart {
                width: parent.width / 2
                height: width
                chartType: Charts.ChartType.LINE
                Component.onCompleted: chartData = {
                                           // See http://www.chartjs.org/docs/#line-chart-data-structure for docs
                                           labels: ["January","February","March","April","May","June","July"],
                                         datasets: [{
                                                    fillColor: "rgba(220,220,220,0.5)",
                                                  strokeColor: "rgba(220,220,220,1)",
                                                   pointColor: "rgba(220,220,220,1)",
                                             pointStrokeColor: "#ffffff",
                                                         data: [65,59,90,81,56,55,40]
                                         }, {
                                                    fillColor: "rgba(151,187,205,0.5)",
                                                  strokeColor: "rgba(151,187,205,1)",
                                                   pointColor: "rgba(151,187,205,1)",
                                             pointStrokeColor: "#ffffff",
                                                         data: [28,48,40,19,96,27,100]
                                         }]
                                     }
            }
        }
    }
}

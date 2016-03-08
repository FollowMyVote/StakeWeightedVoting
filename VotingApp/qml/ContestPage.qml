import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0

import VPlayApps 1.0

import QtQmlTricks.UiElements 2.0

import FollowMyVote.StakeWeightedVoting 1.0

Page {
    title: qsTr("Contest Results")

    property VotingSystem votingSystem
    property alias contest: delegate.displayContest

    Flickable {
        anchors.fill: parent
        anchors.margins: window.dp(16)
        interactive: true
        contentWidth: width
        contentHeight: contestColumn.height

        Column {
            id: contestColumn
            width: parent.width

            ContestDelegate {
                id: delegate

                ExtraAnchors.horizontalFill: parent
                onCastButtonClicked: votingSystem.castCurrentDecision(displayContest)
                onCancelButtonClicked: votingSystem.cancelCurrentDecision(displayContest)
            }
        }
    }
}

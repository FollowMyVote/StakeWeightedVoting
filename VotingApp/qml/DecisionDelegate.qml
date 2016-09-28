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

Label {
    property var decision
    property var contest
    property string voteAction: {
        var chosenOnes = Object.keys(decision.opinions).filter(function(c) {
            return !!decision.opinions[c]
        })
        if (chosenOnes.length !== 1)
            return qsTr("revoked his/her vote")
        var name = contest.getCandidateName(chosenOnes[0], contest)
        return qsTr("voted for %1").arg(name)
    }
    text: qsTr("On %1, %2 %3").arg(decision.timestamp).arg(decision.voter).arg(voteAction)
}

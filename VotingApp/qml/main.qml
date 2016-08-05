import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.1
import QtQml.StateMachine 1.0
import QtGraphicalEffects 1.0
import Qt.labs.settings 1.0
import QtQmlTricks.UiElements 2.0 as UI
import FollowMyVote.StakeWeightedVoting 1.0
import QuickPromise 1.0

ApplicationWindow {
    id: window
    visible: true

    ConnectionProgressPopup {
        visible: !globalStateMachine.connectionStateMachine.connectedState.active
    }

    NavigationDrawer {
        id: _navigationDrawer
        height: window.height
        width: 300
        votingSystem: _votingSystem
    }
    StackView {
        id: mainStack
        anchors.fill: parent

        initialItem: FeedPage {
            id: _feedPage
            navigationDrawer: _navigationDrawer
            votingSystem: _votingSystem
            feedPageState: globalStateMachine.appStateMachine.feedPageState
            connectionStateMachine: globalStateMachine.connectionStateMachine
        }
    }

    Settings {
        property alias x: window.x
        property alias y: window.y
        property alias width: window.width
        property alias height: window.height
    }
    VotingSystem {
        id: _votingSystem
    }
    GlobalStateMachine {
        id: globalStateMachine
        running: true
        feedPage: _feedPage
        votingSystem: _votingSystem
    }
}

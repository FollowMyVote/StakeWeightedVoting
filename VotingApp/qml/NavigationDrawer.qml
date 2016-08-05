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

Drawer {
    id: navigationDrawer

    property var votingSystem

    Menu {
        id: accountSelectorPopup
        Column {
            Repeater {
                model: votingSystem.myAccounts
                delegate: MenuItem {
                    text: qsTr(model.name)
                    highlighted: votingSystem.currentAccount? model.name === votingSystem.currentAccount.name : false
                    onTriggered: {
                        accountSelectorPopup.close()
                        votingSystem.setCurrentAccount(model.name)
                    }
                }
            }
        }
    }
    MenuItem {
        text: qsTr("Current Account:\n%1").arg(votingSystem.currentAccount? votingSystem.currentAccount.name
                                                                          : qsTr("No Account"))
        UI.ExtraAnchors.horizontalFill: parent
        onTriggered: accountSelectorPopup.open()
    }
}

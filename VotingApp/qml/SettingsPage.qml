import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4

import VPlayApps 1.0

import FollowMyVote.StakeWeightedVoting 1.0

ListPage {
    model: [
        {
            "text": qsTr("Current Account"),
            "detailText": qsTr("Account to cast votes and pay with"),
            "id": "account"
        }
    ]

    property VotingSystem votingSystem

    onItemSelected: {
        if (item.id === "account") {
            accountSelector.open()
        }
    }

    Dialog {
        id: accountSelector
        title: qsTr("Select an Account")
        positiveAction: false
        negativeAction: false
        Keys.onEscapePressed: close()
        onIsOpenChanged: if (isOpen) forceActiveFocus()

        AppListView {
            anchors.fill: parent
            model: votingSystem.myAccounts
            delegate: SimpleRow {
                text: model.name
                active: votingSystem.currentAccount === model.name
                onSelected: {
                    votingSystem.currentAccount = model.name
                    accountSelector.close()
                }
            }
        }
    }
}

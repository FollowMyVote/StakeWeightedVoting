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

Page {
    id: createContestPage
    header: ToolBar {
        ToolButton {
            contentItem: UI.SvgIconLoader {
                icon: "qrc:/icons/navigation/arrow_back.svg"
            }
            onClicked: createContestCanceled()
        }
    }

    property VotingSystem votingSystem
    property var contestCreatorApi

    signal createContestCanceled
    signal contestFormComplete
    signal checkoutCanceled
    signal paymentSent
    signal paymentConfirmed

    onCreateContestCanceled: StackView.view.pop()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8

        TextField {
            id: contestNameField
            placeholderText: qsTr("Contest Name")
            Layout.fillWidth: true
            maximumLength: contestCreatorApi.contestLimits[ContestLimits.NameLength]
        }
        TextField {
            id: contestDescriptionField
            placeholderText: qsTr("Contest Description")
            Layout.fillWidth: true
            maximumLength: contestCreatorApi.contestLimits[ContestLimits.DescriptionHardLength]
        }
        Row {
            spacing: 4
            Label {
                text: qsTr("Coin:")
                anchors.baseline: coinSelector.baseline
            }
            ComboBox {
                id: coinSelector
                model: votingSystem.coins
                textRole: "name"
            }
        }
        ListView {
            Layout.fillHeight: true
            Layout.fillWidth: true
            model: ListModel {
                id: contestantModel
            }
            delegate: Row {
                Column {
                    Label {
                        text: model.name
                        font.bold: true
                        width: 300
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    }
                    Label {
                        text: model.description
                        width: 300
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    }
                }
                ToolButton {
                    contentItem: UI.SvgIconLoader {
                        icon: "qrc:/icons/action/delete_forever.svg"
                    }
                    onClicked: contestantModel.remove(index)
                }
            }
        }
        RowLayout {
            spacing: 4
            TextField {
                id: contestantNameField
                Layout.fillWidth: true
                placeholderText: qsTr("Contestant Name")
                maximumLength: contestCreatorApi.contestLimits[ContestLimits.ContestantNameLength]
            }
            TextField {
                id: contestantDescriptionField
                Layout.fillWidth: true
                placeholderText: qsTr("Contestant Description")
                maximumLength: contestCreatorApi.contestLimits[ContestLimits.ContestantDescriptionHardLength]
            }
            Button {
                text: qsTr("Add Contestant")
                onClicked: contestantModel.append({name: contestantNameField.text,
                                                   description: contestantDescriptionField.text})
            }
        }
        RowLayout {
            spacing: 4
            Item { height: 1; Layout.fillWidth: true }
            Button {
                text: qsTr("Cancel")
                onClicked: createContestCanceled()
            }
            Button {
                text: qsTr("Checkout")
                onClicked: contestFormComplete()
            }
        }
    }
}

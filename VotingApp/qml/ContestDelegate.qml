import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import QtQml 2.2
import QtQml.StateMachine 1.0
import QtGraphicalEffects 1.0
import Qt.labs.settings 1.0
import QtQmlTricks.UiElements 2.0 as UI
import FollowMyVote.StakeWeightedVoting 1.0
import QuickPromise 1.0

Rectangle {
    height: contestMainColumn.height + 8

    Column {
        id: contestMainColumn
        UI.ExtraAnchors.topDock: parent
        anchors.margins: 4
        spacing: 8

        RowLayout {
            width: parent.width

            Column {
                id: contestHeaderLeftColumn

                Label {
                    id: contestCoinLabel
                    text: votingSystem.getCoin(coin).name
                }
                Label {
                    id: contestStartTimeLabel
                    text: startTime.toLocaleString(Qt.locale(), Locale.ShortFormat)
                }
            }
            Item { height: 1; Layout.fillWidth: true }
            Button {
                id: favoriteButton
                implicitHeight: contestHeaderLeftColumn.height
                implicitWidth: height
                property bool favorited: false
                contentItem: UI.SvgIconLoader {
                    icon: favoriteButton.favorited? "qrc:/icons/action/favorite.svg"
                                                  : "qrc:/icons/action/favorite_border.svg"
                }
                background: Item {}
                onClicked: favorited = !favorited
            }
        }
        Label {
            UI.ExtraAnchors.horizontalFill: contestMainColumn
            text: name
            font.weight: Font.Bold
            font.pixelSize: 20
            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
        }
        Label {
            UI.ExtraAnchors.horizontalFill: contestMainColumn
            text: description
            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
        }
        UI.GridContainer {
            UI.ExtraAnchors.horizontalFill: contestMainColumn
            rowSpacing: 4
            colSpacing: 4
            cols: Math.min(Math.floor(width / 200), contestants.count)

            Repeater {
                model: contestants
                Button {
                    text: name + "\n" + description
                }
            }
        }
    }
}

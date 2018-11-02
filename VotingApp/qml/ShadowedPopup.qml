import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

Popup {
    id: addAccountPopup
    width: 375
    height: 200
    background: Rectangle {
        layer.enabled: true
        layer.effect: DropShadow {
            color: "#77000000"
            transparentBorder: true
            radius: 50
            samples: 1 + radius * 2
            verticalOffset: 10
        }
    }
    modal: true
    dim: false
    z: 3

    default property alias contents: clientArea.data
    property alias cancelButton: cancelButton
    property alias acceptButton: acceptButton
    property alias showButtons: buttonRow.visible

    ColumnLayout {
        anchors.fill: parent

        Item {
            id: clientArea
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        Row {
            id: buttonRow
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom
            spacing: 10

            Button {
                id: cancelButton
                text: qsTr("Cancel")
            }
            Button {
                id: acceptButton
                text: qsTr("Accept")
            }
        }
    }
}

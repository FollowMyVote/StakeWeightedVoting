import QtQuick 2.7
import QtQuick.Controls 2.0
import Qt.labs.settings 1.0
import QtGraphicalEffects 1.0

ApplicationWindow {
    id: window
    visible: true
    color: "green"

    Drawer {
        id: navigationDrawer
        width: 300
        height: window.height
    }
    StackView {
        id: mainStack
        anchors.fill: parent

        initialItem: Page {
            header: ToolBar {
                ToolButton {
                    contentItem: Image {
                        id: img
                        sourceSize.width: width
                        sourceSize.height: height
                        source: "qrc:/icons/navigation/menu.svg"
                    }
                    onClicked: navigationDrawer.open()
                }
            }
        }
    }

    Settings {
        property alias x: window.x
        property alias y: window.y
        property alias width: window.width
        property alias height: window.height
    }
}

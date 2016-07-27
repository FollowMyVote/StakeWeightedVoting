import QtQuick 2.1;
import QtQmlTricks.UiElements 2.0;

ComboListDelegate {
    id: base;
    key: modelData;
    value: (modelData ? modelData : "");
    width: implicitWidth;
    height: implicitHeight;
    implicitWidth: lbl.contentWidth;
    implicitHeight: lbl.contentHeight;

    readonly property alias label : lbl;

    TextLabel {
        id: lbl;
        text: base.value;
        emphasis: base.active;
        anchors.verticalCenter: parent.verticalCenter;
    }
}

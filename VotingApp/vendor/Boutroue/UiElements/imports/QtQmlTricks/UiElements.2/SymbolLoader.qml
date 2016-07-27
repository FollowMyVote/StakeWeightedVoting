import QtQuick 2.1;
import QtQmlTricks.UiElements 2.0;

Loader {
    id: base;
    onInstanceChanged: {
        if (instance !== null) {
            instance.size = Qt.binding (function () {
                return base.size;
            });
            instance.color = Qt.binding (function () {
                return (base.enabled ? base.color : Style.colorBorder);
            });
            instance.width = Qt.binding (function () {
                return (base.autoSize ? instance.implicitWidth : base.width);
            });
            instance.height = Qt.binding (function () {
                return (base.autoSize ? instance.implicitHeight : base.height);
            });
        }
    }

    property int   size     : Style.fontSizeNormal;
    property color color    : Style.colorForeground;
    property alias symbol   : base.sourceComponent;
    property bool  autoSize : true;

    readonly property AbstractSymbol instance : item;
}

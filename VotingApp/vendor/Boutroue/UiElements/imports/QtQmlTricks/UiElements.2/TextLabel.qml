import QtQuick 2.1;
import QtQmlTricks.UiElements 2.0;

Text {
    color: (enabled ? Style.colorForeground : Style.colorBorder);
    textFormat: Text.PlainText;
    renderType: (Style.useNativeText ? Text.NativeRendering : Text.QtRendering);
    verticalAlignment: Text.AlignVCenter;
    font {
        family: Style.fontName;
        weight: (emphasis ? Font.Bold : (Style.useSlimFonts ? Font.Light : Font.Normal));
        pixelSize: Style.fontSizeNormal;
    }

    property bool emphasis : false;
}

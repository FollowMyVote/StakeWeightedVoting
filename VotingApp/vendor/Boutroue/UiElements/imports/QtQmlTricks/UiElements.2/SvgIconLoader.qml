import QtQuick 2.1;
import QtQmlTricks.UiElements 2.0;

PixelPerfectContainer {
    id: base;
    width: implicitWidth;
    height: implicitHeight;
    visible: (icon !== "");
    contentItem: img;
    implicitWidth: (helper.size * helper.horizontalRatio);
    implicitHeight: (helper.size * helper.verticalRatio);

    property int   size            : Style.iconSize (1);
    property color color           : Style.colorNone;
    property alias icon            : helper.icon;
    property alias verticalRatio   : helper.verticalRatio;
    property alias horizontalRatio : helper.horizontalRatio;

    Image {
        id: img;
        cache: !helper.live;
        width: helper.size;
        height: helper.size;
        smooth: false;
        opacity: (enabled ? 1.0 : 0.65);
        fillMode: Image.Pad;
        antialiasing: false;
        asynchronous: true;

        SvgIconHelper on source {
            id: helper;
            size: base.size;
            color: (base.enabled ? base.color : Style.colorBorder);
        }
    }
}

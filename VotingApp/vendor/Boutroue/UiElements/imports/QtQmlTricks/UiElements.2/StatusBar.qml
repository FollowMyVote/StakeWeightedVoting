import QtQuick 2.1;
import QtQmlTricks.UiElements 2.0;

Rectangle {
    id: statusbar;
    height: (layout.height + layout.anchors.margins * 2);
    gradient: Style.gradientIdle (Style.colorWindow);
    ExtraAnchors.bottomDock: parent;

    default property alias content : layout.data;

    Line { ExtraAnchors.topDock: parent; }
    StretchRowContainer {
        id: layout;
        spacing: Style.spacingNormal;
        anchors.margins: Style.spacingNormal;
        anchors.verticalCenter: parent.verticalCenter;
        ExtraAnchors.horizontalFill: parent;

        // NOTE : CONTENT GOES HERE
    }
}

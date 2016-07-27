import QtQuick 2.1;
import QtQmlTricks.UiElements 2.0;

Rectangle {
    id: toolbar;
    height: (layout.height + layout.anchors.margins * 2);
    gradient: Style.gradientIdle (Style.colorWindow);
    ExtraAnchors.topDock: parent;

    default property alias content : layout.data;

    Line { ExtraAnchors.bottomDock: parent; }
    StretchRowContainer {
        id: layout;
        spacing: Style.spacingNormal;
        anchors.margins: Style.spacingNormal;
        anchors.verticalCenter: parent.verticalCenter;
        ExtraAnchors.horizontalFill: parent;

        // NOTE : CONTENT GOES HERE
    }
}

import QtQuick 2.1;
import QtQmlTricks.UiElements 2.0;

Item {
    id: base;

    property int wheelResolution : 1200; // NOTE : change if zoom is too slow / too fast

    property bool enableMoving   : true;
    property bool enableZooming  : true;
    property bool showIndicators : true;

    property real contentZoom    : 1;
    property real contentZoomMin : 0.05;
    property real contentZoomMax : 50;
    property real contentPadding : 0;

    property Item contentItem : null;

    default property alias content : base.contentItem;

    function ensureVisible (item) {
        if (item && contentItem) {
            var localPos = item.mapToItem (contentItem, item.width / 2, item.height / 2);
            var idealContentOffsetX = (((contentItem.width  / 2) - localPos.x) * contentZoom);
            var idealContentOffsetY = (((contentItem.height / 2) - localPos.y) * contentZoom);
            priv.contentOffsetX = priv.clamp (idealContentOffsetX, -priv.contentOffsetXLimit, +priv.contentOffsetXLimit);
            priv.contentOffsetY = priv.clamp (idealContentOffsetY, -priv.contentOffsetYLimit, +priv.contentOffsetYLimit);
        }
    }

    Timer {
        id: timer;
        repeat: false;
        running: false;
        interval: 1000;
        onTriggered: { priv.motion = false; }
    }
    QtObject {
        id: priv;

        property bool motion : false;

        property real lastPosX : 0;
        property real lastPosY : 0;

        property real contentOffsetX : 0;
        property real contentOffsetY : 0;

        readonly property real contentZoomedWidth  : (contentItem ? contentItem.width  * contentZoom : 0);
        readonly property real contentZoomedHeight : (contentItem ? contentItem.height * contentZoom : 0);

        readonly property real contentOffsetXLimit : ((contentZoomedWidth  / 2) - (width  / 2) + contentPadding);
        readonly property real contentOffsetYLimit : ((contentZoomedHeight / 2) - (height / 2) + contentPadding);

        readonly property real positionRatioX : (contentOffsetX + contentOffsetXLimit) / (contentOffsetXLimit * 2);
        readonly property real positionRatioY : (contentOffsetY + contentOffsetYLimit) / (contentOffsetYLimit * 2);

        readonly property real sizeRatioX : (width  / (contentZoomedWidth  + contentPadding * 2));
        readonly property real sizeRatioY : (height / (contentZoomedHeight + contentPadding * 2));

        function clamp (value, min, max) {
            return (min !== undefined && value < min
                    ? min
                    : (max !== undefined && value > max
                       ? max
                       : value));
        }

        function applyZoom (zoom) {
            contentZoom = clamp (zoom, contentZoomMin, contentZoomMax);
        }

        function applyOffsetX (offset) {
            contentOffsetX = ((contentZoomedWidth + contentPadding * 2) > width
                              ? clamp (offset, -contentOffsetXLimit, contentOffsetXLimit)
                              : 0);
        }

        function applyOffsetY (offset) {
            contentOffsetY = ((contentZoomedHeight + contentPadding * 2) > height
                              ? clamp (offset, -contentOffsetYLimit, contentOffsetYLimit)
                              : 0);
        }

        function doRecordMotion () {
            motion = true;
            timer.restart ();
        }

        function doSavePos (posX, posY) {
            if (enableMoving) {
                doRecordMotion ();
                lastPosX = posX;
                lastPosY = posY;
            }
        }

        function doZoomAtPos (posX, posY, zoomDelta) {
            if (enableZooming) {
                doRecordMotion ();
                var posOnContentBeforeZoom = mapToItem (contentItem, posX, posY);
                applyZoom (contentZoom + (contentZoom * (zoomDelta / wheelResolution)));
                var posOnContentAfterZoom = mapToItem (contentItem, posX, posY);
                applyOffsetX (contentOffsetX + (posOnContentAfterZoom.x - posOnContentBeforeZoom.x) * contentZoom);
                applyOffsetY (contentOffsetY + (posOnContentAfterZoom.y - posOnContentBeforeZoom.y) * contentZoom);
            }
        }

        function doMoveToPos (posX, posY) {
            if (enableMoving) {
                doRecordMotion ();
                applyOffsetX (contentOffsetX + posX - lastPosX);
                applyOffsetY (contentOffsetY + posY - lastPosY);
                doSavePos (posX, posY);
            }
        }
    }
    MouseArea {
        anchors.fill: parent;
        onWheel: { priv.doZoomAtPos (wheel.x, wheel.y, wheel.angleDelta.y); }
        onPressed: { priv.doSavePos (mouse.x, mouse.y); }
        onPositionChanged: { priv.doMoveToPos (mouse.x, mouse.y); }

    }
    Item {
        id: container;
        clip: true;
        children: contentItem;
        states: State {
            when: (contentItem !== null);

            PropertyChanges {
                target: contentItem;
                scale: contentZoom;
                anchors {
                    verticalCenterOffset: priv.contentOffsetY;
                    horizontalCenterOffset: priv.contentOffsetX;
                }
            }
            AnchorChanges {
                target: contentItem;
                anchors {
                    verticalCenter: container.verticalCenter;
                    horizontalCenter: container.horizontalCenter;
                }
            }
        }
        anchors.fill: parent;
    }
    Rectangle {
        id: indicatorY;
        y: (roomY - roomY * priv.positionRatioY);
        color: Style.colorSelection;
        width: Style.spacingNormal;
        height: Math.max (base.height * priv.sizeRatioY, Style.spacingBig * 2);
        visible: (showIndicators && height < parent.height);
        opacity: (priv.motion ? 1.0 : 0.0);
        anchors.right: parent.right;

        readonly property real roomY : (base.height - height);

        Behavior on opacity {
            NumberAnimation {
                duration: 250;
            }
        }
    }
    Rectangle {
        id: indicatorX;
        x: (roomX - roomX * priv.positionRatioX);
        color: Style.colorSelection;
        width: Math.max (base.width * priv.sizeRatioX, Style.spacingBig * 2);
        height: Style.spacingNormal;
        visible: (showIndicators && width < parent.width);
        opacity: (priv.motion ? 1.0 : 0.0);
        anchors.bottom: parent.bottom;

        readonly property real roomX : (base.width - width);

        Behavior on opacity {
            NumberAnimation {
                duration: 250;
            }
        }
    }
    Column {
        spacing: Style.spacingSmall;
        visible: showIndicators;
        opacity: (priv.motion ? 1.0 : 0.0);
        anchors.margins: Style.spacingNormal;
        ExtraAnchors.topLeftCorner: parent;

        Behavior on opacity {
            NumberAnimation {
                duration: 250;
            }
        }
        Rectangle {
            id: zoomScale;
            color: Style.colorSelection;
            width: (Style.spacingBig * 4);
            height: (Style.lineSize * 2);
            visible: showIndicators;
            opacity: (priv.motion ? 1.0 : 0.0);

            Rectangle {
                id: zoomTick;
                x: ((parent.width - width) * (contentZoom - contentZoomMin) / (contentZoomMax - contentZoomMin));
                color: Style.colorSelection;
                width: (Style.lineSize * 2);
                height: Style.spacingNormal;
                anchors.verticalCenter: parent.verticalCenter;
            }
        }
        TextLabel {
            id: lblZoomPercent;
            text: "%1 %".arg ((contentZoom * 100).toFixed (2));
            color: Style.colorSelection;
            style: Text.Outline;
            styleColor: Style.colorEditable;
            font.bold: true;
            font.pixelSize: Style.fontSizeTitle;
        }
    }
}

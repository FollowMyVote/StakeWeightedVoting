import QtQuick 2.1;
import QtQmlTricks.UiElements 2.0;

ProgressJauge {
    id: base;
    width: implicitWidth;
    height: implicitHeight;
    barSize: Style.spacingNormal;
    implicitWidth: 200;
    implicitHeight: handle.height;

    property int  decimals : 0;

    property int handleSize : (Style.spacingBig * 2);

    signal edited ();

    MouseArea {
        anchors.fill: parent;
        onClicked: {
            var tmp = Style.convert (mouse.x,
                                     0,
                                     width,
                                     minValue,
                                     maxValue);
            value = parseFloat (tmp.toFixed (decimals));
            edited ();
        }
    }
    Rectangle {
        id: handle;
        width: handleSize;
        height: handleSize;
        radius: (handleSize / 2);
        enabled: base.enabled;
        antialiasing: radius;
        gradient: (enabled
                   ? (clicker.pressed
                      ? Style.gradientPressed ()
                      : Style.gradientIdle (Qt.lighter (Style.colorClickable, clicker.containsMouse ? 1.15 : 1.0)))
                   : Style.gradientDisabled ());
        border {
            width: Style.lineSize;
            color: Style.colorBorder;
        }
        anchors.verticalCenter: parent.verticalCenter;

        Binding on x {
            when: !clicker.pressed;
            value: Style.convert (base.value,
                                  base.minValue,
                                  base.maxValue,
                                  clicker.drag.minimumX,
                                  clicker.drag.maximumX);
        }
        MouseArea {
            id: clicker;
            drag {
                target: handle;
                minimumX: 0;
                maximumX: (base.width - handle.width);
                minimumY: 0;
                maximumY: 0;
            }
            enabled: base.enabled;
            hoverEnabled: Style.useHovering;
            anchors.fill: parent;
            onPressed: {
                if (tooltip === null) {
                    tooltip = compoTooltip.createObject (Introspector.window (base));
                }
            }
            onReleased: {
                if (tooltip !== null) {
                    tooltip.destroy ();
                    tooltip = null;
                }
            }
            onPositionChanged: {
                if (pressed) {
                    var tmp = Style.convert (handle.x,
                                             clicker.drag.minimumX,
                                             clicker.drag.maximumX,
                                             minValue,
                                             maxValue);
                    value = parseFloat (tmp.toFixed (decimals));
                    edited ();
                }
            }

            property Balloon tooltip : null;

            Component {
                id: compoTooltip;

                Balloon {
                    x: (handleTopCenterAbsPos.x - width / 2);
                    y: (handleTopCenterAbsPos.y - height - Style.spacingNormal);
                    z: 9999999;
                    content: base.value.toFixed (decimals);

                    readonly property var handleTopCenterAbsPos : base.mapToItem (parent,
                                                                                  (handle.x + handle.width / 2),
                                                                                  (handle.y));
                }
            }
        }
    }
}

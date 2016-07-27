import QtQuick 2.0;
import QtQmlTricks.UiElements 2.0;

Item {
    id: base;
    width: implicitWidth;
    height: implicitHeight;
    implicitWidth: diameter;
    implicitHeight: diameter;

    property int   sides       : 3;
    property int   angle       : 0;
    property int   diameter    : 0;
    property int   strokeSize  : 0;
    property color strokeColor : Style.colorForeground;
    property color fillColor   : Style.colorForeground;

    readonly property int minSides : 3;
    readonly property int maxSides : 360;

    readonly property point center : Qt.point (width / 2, height / 2);

    Polygon {
        id: polygon;
        color: fillColor;
        stroke: strokeColor;
        border: strokeSize;
        closed: true;
        points: {
            var ret = [];
            if (sides >= minSides && sides <= maxSides) {
                for (var idx = 0; idx < sides; idx++) {
                    var curr = deg2rad (angle + (idx * 360 / sides));
                    ret.push (Qt.point ((center.x + diameter * Math.cos (curr) / 2 ),
                                        (center.y + diameter * Math.sin (curr) / 2 )))
                }
            }
            return ret;
        }

        function deg2rad (deg) {
            return (deg * Math.PI / 180);
        }
    }
}

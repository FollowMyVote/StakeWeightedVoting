import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import Qt.labs.settings 1.0

import VPlayApps 1.0

import FollowMyVote.StakeWeightedVoting 1.0

Canvas {
    id: rectCanvas
    width: parent.width - window.dp(16)
    x: window.dp(8)
    z: 10
    height: window.dp(220)
    contextType: "2d"
    onPaint: {
        var ctx = rectCanvas.getContext("2d")
        if (!ctx) {
            console.log("No canvas context!")
            return
        }

        ctx.dashedLineTo = function (fromX, fromY, toX, toY, pattern) {
            // This function taken from:
            // https://davidowens.wordpress.com/2010/09/07/html-5-canvas-and-dashed-lines/
            // Our growth rate for our line can be one of the following:
            //   (+,+), (+,-), (-,+), (-,-)
            // Because of this, our algorithm needs to understand if the x-coord and
            // y-coord should be getting smaller or larger and properly cap the values
            // based on (x,y).
            var lt = function (a, b) { return a <= b; };
            var gt = function (a, b) { return a >= b; };
            var capmin = function (a, b) { return Math.min(a, b); };
            var capmax = function (a, b) { return Math.max(a, b); };

            var checkX = { thereYet: gt, cap: capmin };
            var checkY = { thereYet: gt, cap: capmin };

            if (fromY - toY > 0) {
                checkY.thereYet = lt;
                checkY.cap = capmax;
            }
            if (fromX - toX > 0) {
                checkX.thereYet = lt;
                checkX.cap = capmax;
            }

            this.moveTo(fromX, fromY);
            var offsetX = fromX;
            var offsetY = fromY;
            var idx = 0, dash = true;
            while (!(checkX.thereYet(offsetX, toX) && checkY.thereYet(offsetY, toY))) {
                var ang = Math.atan2(toY - fromY, toX - fromX);
                var len = pattern[idx];

                offsetX = checkX.cap(toX, offsetX + (Math.cos(ang) * len));
                offsetY = checkY.cap(toY, offsetY + (Math.sin(ang) * len));

                if (dash) this.lineTo(offsetX, offsetY);
                else this.moveTo(offsetX, offsetY);

                idx = (idx + 1) % pattern.length;
                dash = !dash;
            }
        };

        var pattern = [window.dp(20), window.dp(20)]
        var margin = window.dp(5)
        ctx.globalAlpha = 0.7
        ctx.strokeStyle = "#777777"
        ctx.fillStyle = Theme.backgroundColor
        ctx.lineWidth = margin
        ctx.fillRect(0,0,width, height)
        ctx.beginPath()
        ctx.dashedLineTo(margin,margin,margin,height-margin*2, pattern)
        ctx.dashedLineTo(margin,height-margin,width-margin,height-margin, pattern)
        ctx.dashedLineTo(width-margin,height-margin,width-margin,margin, pattern)
        ctx.dashedLineTo(width-margin,margin,margin,margin, pattern)
        ctx.stroke()
    }

    signal clicked

    AppText {
        anchors.centerIn: parent
        text: qsTr("Create Poll")
        font.pixelSize: window.sp(24)
        font.weight: Font.Bold
    }
    MouseArea {
        anchors.fill: parent
        onClicked: rectCanvas.clicked()
    }
}

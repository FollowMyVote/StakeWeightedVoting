import QtQuick 2.1;
import QtQmlTricks.UiElements 2.0;

Item {
    id: base;

    property int   repeatDelay    : 650;
    property int   repeatInterval : 30;
    property bool  autoRepeat     : false;
    property alias pressed        : clicker.pressed;
    property alias hoverEnabled   : clicker.hoverEnabled;
    property alias containsMouse  : clicker.containsMouse;

    signal clicked (bool isRepeated);

    MouseArea {
        id: clicker;
        anchors.fill: parent;
        onPressed: {
            if (autoRepeat) {
                timerAutoRepeatDelay.start ();
            }
        }
        onReleased: {
            if (autoRepeat) {
                if (timerAutoRepeatDelay.running) {
                    timerAutoRepeatDelay.stop ();
                    base.clicked (false);
                }
                else {
                    timerAutoRepeatInterval.stop ();
                }
            }
            else {
                base.clicked (false);
            }
        }

        Timer {
            id: timerAutoRepeatDelay;
            repeat: false;
            running: false;
            interval: repeatDelay;
            onTriggered: { timerAutoRepeatInterval.start (); }
        }
        Timer {
            id: timerAutoRepeatInterval;
            repeat: true;
            running: false;
            interval: repeatInterval;
            triggeredOnStart: true;
            onTriggered: { base.clicked (true); }
        }
    }
}

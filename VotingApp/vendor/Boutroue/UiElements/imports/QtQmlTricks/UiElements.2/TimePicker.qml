import QtQuick 2.1;
import QtQmlTricks.UiElements 2.0;

Item {
    id: base;
    width: implicitWidth;
    height: implicitHeight;
    implicitWidth: layout.implicitWidth;
    implicitHeight: layout.implicitHeight;

    property int hours   : 0;
    property int minutes : 0;
    property int seconds : 0;

    QtObject {
        id: priv;

        function intFromStr (str) {
            var tmp = parseInt (str.trim ());
            return (!isNaN (tmp) ? tmp : 0);
        }
    }
    TextBox {
        id: metrics;
        text: "000";
        opacity: 0.0;
    }
    StretchRowContainer {
        id: layout;
        spacing: Style.spacingSmall;
        anchors.centerIn: parent;

        StretchColumnContainer {
            spacing: Style.spacingSmall;
            anchors.verticalCenter: parent.verticalCenter;

            SymbolLoader {
                size: Style.fontSizeNormal;
                color: (enabled
                        ? (clickerIncrHours.containsMouse
                           ? Style.colorLink
                           : Style.colorForeground)
                        : Style.colorBorder);
                symbol: Style.symbolArrowUp;
                anchors.horizontalCenter: parent.horizontalCenter;

                AutoRepeatableClicker {
                    id: clickerIncrHours;
                    autoRepeat: true;
                    hoverEnabled: Style.useHovering;
                    anchors.fill: parent;
                    onClicked: { hours = (hours < 23 ? hours +1 : 0); }
                }
            }
            TextBox {
                id: inputHours;
                textAlign: TextInput.AlignHCenter;
                validator: IntValidator { top: 23; bottom: 0; }
                implicitWidth: metrics.width;
                anchors.horizontalCenter: parent.horizontalCenter;
                onActiveFocusChanged: {
                    if (!activeFocus) {
                        apply ();
                    }
                }
                Keys.onReturnPressed: {
                    apply ();
                    focus = false;
                }
                Keys.onEnterPressed: {
                    apply ();
                    focus = false;
                }
                Keys.onUpPressed: {
                    hours = (hours < 23 ? hours +1 : 0);
                }
                Keys.onDownPressed: {
                    hours = (hours > 0 ? hours -1 : 23);
                }
                KeyNavigation.tab: inputMinutes;
                KeyNavigation.backtab: inputSeconds;

                function apply () {
                    hours = priv.intFromStr (text);
                }

                Binding on text { value: (hours < 10 ? "0%1" : "%1").arg (hours); }
            }
            SymbolLoader {
                size: Style.fontSizeNormal;
                color: (enabled
                        ? (clickerDecrHours.containsMouse
                           ? Style.colorLink
                           : Style.colorForeground)
                        : Style.colorBorder);
                symbol: Style.symbolArrowDown;
                anchors.horizontalCenter: parent.horizontalCenter;

                AutoRepeatableClicker {
                    id: clickerDecrHours;
                    autoRepeat: true;
                    hoverEnabled: Style.useHovering;
                    anchors.fill: parent;
                    onClicked: { hours = (hours > 0 ? hours -1 : 23); }
                }
            }
        }
        TextLabel {
            text: ":";
            emphasis: true;
            anchors.verticalCenter: parent.verticalCenter;
        }
        StretchColumnContainer {
            spacing: Style.spacingSmall;
            anchors.verticalCenter: parent.verticalCenter;

            SymbolLoader {
                size: Style.fontSizeNormal;
                color: (enabled
                        ? (clickerIncrMinutes.containsMouse
                           ? Style.colorLink
                           : Style.colorForeground)
                        : Style.colorBorder);
                symbol: Style.symbolArrowUp;
                anchors.horizontalCenter: parent.horizontalCenter;

                AutoRepeatableClicker {
                    id: clickerIncrMinutes;
                    autoRepeat: true;
                    hoverEnabled: Style.useHovering;
                    anchors.fill: parent;
                    onClicked: { minutes = (minutes < 59 ? minutes +1 : 0); }
                }
            }
            TextBox {
                id: inputMinutes;
                textAlign: TextInput.AlignHCenter;
                validator: IntValidator { top: 59; bottom: 0; }
                implicitWidth: metrics.width;
                anchors.horizontalCenter: parent.horizontalCenter;
                onActiveFocusChanged: {
                    if (!activeFocus) {
                        apply ();
                    }
                }
                Keys.onReturnPressed: {
                    apply ();
                    focus = false;
                }
                Keys.onEnterPressed: {
                    apply ();
                    focus = false;
                }
                Keys.onUpPressed: {
                    minutes = (minutes < 59 ? minutes +1 : 0);
                }
                Keys.onDownPressed: {
                    minutes = (minutes > 0 ? minutes -1 : 59);
                }
                KeyNavigation.tab: inputSeconds;
                KeyNavigation.backtab: inputHours;

                function apply () {
                    minutes = priv.intFromStr (text);
                }

                Binding on text { value: (minutes < 10 ? "0%1" : "%1").arg (minutes); }
            }
            SymbolLoader {
                size: Style.fontSizeNormal;
                color: (enabled
                        ? (clickerDecrMinutes.containsMouse
                           ? Style.colorLink
                           : Style.colorForeground)
                        : Style.colorBorder);
                symbol: Style.symbolArrowDown;
                anchors.horizontalCenter: parent.horizontalCenter;

                AutoRepeatableClicker {
                    id: clickerDecrMinutes;
                    autoRepeat: true;
                    hoverEnabled: Style.useHovering;
                    anchors.fill: parent;
                    onClicked: { minutes = (minutes > 0 ? minutes -1 : 59); }
                }
            }
        }
        TextLabel {
            text: ":";
            emphasis: true;
            anchors.verticalCenter: parent.verticalCenter;
        }
        StretchColumnContainer {
            spacing: Style.spacingSmall;
            anchors.verticalCenter: parent.verticalCenter;

            SymbolLoader {
                size: Style.fontSizeNormal;
                color: (enabled
                        ? (clickerIncrSecondes.containsMouse
                           ? Style.colorLink
                           : Style.colorForeground)
                        : Style.colorBorder);
                symbol: Style.symbolArrowUp;
                anchors.horizontalCenter: parent.horizontalCenter;

                AutoRepeatableClicker {
                    id: clickerIncrSecondes;
                    autoRepeat: true;
                    hoverEnabled: Style.useHovering;
                    anchors.fill: parent;
                    onClicked: { seconds = (seconds < 59 ? seconds +1 : 0); }
                }
            }
            TextBox {
                id: inputSeconds;
                textAlign: TextInput.AlignHCenter;
                validator: IntValidator { top: 59; bottom: 0; }
                implicitWidth: metrics.width;
                anchors.horizontalCenter: parent.horizontalCenter;
                onActiveFocusChanged: {
                    if (!activeFocus) {
                        apply ();
                    }
                }
                Keys.onReturnPressed: {
                    apply ();
                    focus = false;
                }
                Keys.onEnterPressed: {
                    apply ();
                    focus = false;
                }
                Keys.onUpPressed: {
                    seconds = (seconds < 59 ? seconds +1 : 0);
                }
                Keys.onDownPressed: {
                    seconds = (seconds > 0 ? seconds -1 : 59);
                }
                KeyNavigation.tab: inputHours;
                KeyNavigation.backtab: inputMinutes;

                function apply () {
                    seconds = priv.intFromStr (text);
                }

                Binding on text { value: (seconds < 10 ? "0%1" : "%1").arg (seconds); }
            }
            SymbolLoader {
                size: Style.fontSizeNormal;
                color: (enabled
                        ? (clickerDecrSeconds.containsMouse
                           ? Style.colorLink
                           : Style.colorForeground)
                        : Style.colorBorder);
                symbol: Style.symbolArrowDown;
                anchors.horizontalCenter: parent.horizontalCenter;

                AutoRepeatableClicker {
                    id: clickerDecrSeconds;
                    autoRepeat: true;
                    hoverEnabled: Style.useHovering;
                    anchors.fill: parent;
                    onClicked: { seconds = (seconds > 0 ? seconds -1 : 59); }
                }
            }
        }
    }
}

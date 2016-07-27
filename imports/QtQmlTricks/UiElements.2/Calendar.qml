import QtQuick 2.2;
import QtQmlTricks.UiElements 2.0;

Item {
    id: base;
    width: implicitWidth;
    height: implicitHeight;
    implicitWidth: layout.implicitWidth;
    implicitHeight: (rowYear.implicitHeight +
                     rowMonth.implicitHeight +
                     rowWeekdays.implicitHeight +
                     gridDays.implicitHeight);
    onValueChanged: {
        currentYear = value.getFullYear ();
        currentMonth = (value.getMonth() +1);
    }

    property date value : new Date;

    property int currentYear  : (value.getFullYear ());
    property int currentMonth : (value.getMonth () +1);

    readonly property var days : {
        var ret = [];
        var currMonthDays = getDaysOfMonth (currentMonth, currentYear);
        var prevMonthDays = (currentMonth > 1
                             ? getDaysOfMonth (currentMonth -1, currentYear)
                             : getDaysOfMonth (12, currentYear -1));
        var nextMonthDays = (currentMonth < 12
                             ? getDaysOfMonth (currentMonth +1, currentYear)
                             : getDaysOfMonth (1, currentYear +1));
        var firstDay = new Date (currentYear, currentMonth -1, 1);
        var dayOfWeek = (firstDay.getDay () > 0 ? firstDay.getDay () : 7);
        var prevCount = (dayOfWeek -1);
        var missing = ((6 * 7) - currMonthDays - prevCount);
        var idx;
        for (idx = 0; idx < prevCount; idx++) {
            ret.push (currentMonth > 1
                      ? new Date (currentYear, currentMonth -2, prevMonthDays - prevCount + idx +1)
                      : new Date (currentYear -1, 11, prevMonthDays - prevCount + idx +1));
        }
        for (idx = 0; idx < currMonthDays; idx++) {
            ret.push (new Date (currentYear, currentMonth -1, idx +1));
        }
        for (idx = 0; idx < missing; idx++) {
            ret.push (currentMonth < 12
                      ? new Date (currentYear, currentMonth, idx +1)
                      : new Date (currentYear +1, 0, idx +1));
        }
        return ret;
    }

    function getDaysOfMonth (month, year) {
        switch (month) {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            return 31;
        case 4:
        case 6:
        case 9:
        case 11:
            return 30;
        case 2:
            return (year % 4 === 0 && year % 100 !== 0 ? 29 : 28);
        default:
            return -1;
        }
    }

    StretchColumnContainer {
        id: layout;
        spacing: Style.spacingNormal;
        anchors.fill: parent;

        StretchRowContainer {
            id: rowYear;
            spacing: Style.spacingNormal;

            SymbolLoader {
                size: Style.fontSizeNormal;
                color: (enabled
                        ? (clickerPrevYear.containsMouse
                           ? Style.colorLink
                           : Style.colorForeground)
                        : Style.colorBorder);
                symbol: Style.symbolArrowLeft;
                anchors.verticalCenter: parent.verticalCenter;

                MouseArea {
                    id: clickerPrevYear;
                    hoverEnabled: Style.useHovering;
                    anchors.fill: parent;
                    onClicked: { currentYear--; }
                }
            }
            Stretcher { }
            MouseArea {
                implicitWidth: editYear.implicitWidth;
                implicitHeight: editYear.implicitHeight
                anchors.verticalCenter: parent.verticalCenter;
                onClicked: {
                    editYear.visible = true;
                    editYear.forceActiveFocus ();
                }

                TextLabel {
                    visible: !editYear.visible;
                    anchors.centerIn: parent;

                    Binding on text { value: currentYear; }
                }
                NumberBox {
                    id: editYear;
                    visible: false;
                    minValue: -3000;
                    maxValue: +3000;
                    showButtons: false;
                    anchors.centerIn: parent;
                    onValueChanged: {
                        currentYear = value;
                        focus = false;
                        visible = false;
                    }

                    Binding on value { value: currentYear; }
                }
            }
            Stretcher { }
            SymbolLoader {
                size: Style.fontSizeNormal;
                color: (enabled
                        ? (clickerNextYear.containsMouse
                           ? Style.colorLink
                           : Style.colorForeground)
                        : Style.colorBorder);
                symbol: Style.symbolArrowRight;
                anchors.verticalCenter: parent.verticalCenter;

                MouseArea {
                    id: clickerNextYear;
                    hoverEnabled: Style.useHovering;
                    anchors.fill: parent;
                    onClicked: { currentYear++; }
                }
            }
        }
        StretchRowContainer {
            id: rowMonth;
            spacing: Style.spacingNormal;

            SymbolLoader {
                size: Style.fontSizeNormal;
                color: (enabled
                        ? (clickerPrevMonth.containsMouse
                           ? Style.colorLink
                           : Style.colorForeground)
                        : Style.colorBorder);
                symbol: Style.symbolArrowLeft;
                anchors.verticalCenter: parent.verticalCenter;

                MouseArea {
                    id: clickerPrevMonth;
                    hoverEnabled: Style.useHovering;
                    anchors.fill: parent;
                    onClicked: { currentMonth = (currentMonth > 1 ? currentMonth -1 : 12); }
                }
            }
            Stretcher { }
            TextLabel {
                text: Qt.locale ().monthName (currentMonth -1, Locale.LongFormat);
                horizontalAlignment: Text.AlignHCenter;
                font.pixelSize: Style.fontSizeBig;
                anchors.verticalCenter: parent.verticalCenter;
            }
            Stretcher { }
            SymbolLoader {
                size: Style.fontSizeNormal;
                color: (enabled
                        ? (clickerNextMonth.containsMouse
                           ? Style.colorLink
                           : Style.colorForeground)
                        : Style.colorBorder);
                symbol: Style.symbolArrowRight;
                anchors.verticalCenter: parent.verticalCenter;

                MouseArea {
                    id: clickerNextMonth;
                    hoverEnabled: Style.useHovering;
                    anchors.fill: parent;
                    onClicked: { currentMonth = (currentMonth < 12 ? currentMonth +1 : 1); }
                }
            }
        }
        GridContainer {
            id: rowWeekdays;
            cols: capacity;
            capacity: 7;
            colSpacing: Style.spacingNormal;

            Repeater {
                model: [
                    Qt.Monday,
                    Qt.Tuesday,
                    Qt.Wednesday,
                    Qt.Thursday,
                    Qt.Friday,
                    Qt.Saturday,
                    Qt.Sunday,
                ];
                delegate: TextLabel {
                    text: Qt.locale ().standaloneDayName (modelData, Locale.ShortFormat);
                    verticalAlignment: Text.AlignVCenter;
                    horizontalAlignment: Text.AlignHCenter;
                    font.pixelSize: Style.fontSizeSmall;
                    anchors.verticalCenter: parent.verticalCenter;
                }
            }
        }
        Stretcher {
            implicitWidth: gridDays.implicitWidth;

            GridContainer {
                id: gridDays;
                cols: 7;
                capacity: (7 * 6);
                colSpacing: Style.spacingNormal;
                rowSpacing: Style.spacingNormal;
                anchors.fill: parent;

                Repeater {
                    model: days;
                    delegate: TextLabel {
                        id: day;
                        color: (enabled
                                ? (isCurrent
                                   ? Style.colorEditable
                                   : (modelData.getMonth () +1 === currentMonth
                                      ? Style.colorForeground
                                      : Style.colorBorder))
                                : Style.colorBorder);
                        text: Qt.formatDate (modelData, "d");
                        verticalAlignment: Text.AlignVCenter;
                        horizontalAlignment: Text.AlignHCenter;

                        readonly property bool isCurrent : (Qt.formatDate (modelData, "yyyy-MM-dd") ===
                                                            Qt.formatDate (value,     "yyyy-MM-dd"));

                        MouseArea {
                            id: clicker;
                            hoverEnabled: Style.useHovering;
                            anchors {
                                fill: parent;
                                margins: (-Style.spacingNormal / 2);
                            }
                            onClicked: {
                                day.forceActiveFocus ();
                                value = modelData;
                            }
                        }
                        Rectangle {
                            z: -1;
                            color: (enabled
                                    ? (clicker.containsMouse
                                       ? Style.opacify ( Style.colorHighlight, 0.65)
                                       : Style.colorSelection)
                                    : Style.colorSecondary);
                            width: size;
                            height: size;
                            radius: (size / 2);
                            visible: (day.isCurrent || clicker.containsMouse );
                            antialiasing: radius;
                            anchors.centerIn: parent;

                            readonly property int size : Math.max (parent.contentHeight,
                                                                   parent.contentWidth) + Style.spacingNormal;
                        }
                    }
                }
            }
        }
    }
}

import QtQuick 2.1;
import QtQmlTricks.UiElements 2.0;

FocusScope {
    id: base;
    width: implicitWidth;
    height: implicitHeight;
    implicitWidth: (input.contentWidth + padding * 2);
    implicitHeight: (input.contentHeight + padding * 2);

    property int   padding    : Style.spacingNormal;
    property alias text       : input.text;
    property alias readOnly   : input.readOnly;
    property alias textFont   : input.font;
    property alias textColor  : input.color;
    property alias textAlign  : input.horizontalAlignment;
    property alias textHolder : holder.text;
    property alias rounding   : rect.radius;

    function selectAll () {
        input.selectAll ();
    }

    function clear () {
        input.text = "";
    }

    Rectangle {
        id: rect;
        radius: Style.roundness;
        enabled: base.enabled;
        visible: !readOnly;
        antialiasing: radius;
        gradient: (enabled ? Style.gradientEditable () : Style.gradientDisabled ());
        border {
            width: (input.activeFocus ? Style.lineSize * 2 : Style.lineSize);
            color: (input.activeFocus ? Style.colorSelection : Style.colorBorder);
        }
        anchors.fill: parent;
    }
    Item {
        clip: (input.contentHeight > input.height);
        enabled: base.enabled;
        anchors {
            fill: rect;
            margins: rect.border.width;
        }

        TextEdit {
            id: input;
            focus: true;
            color: (enabled ? Style.colorForeground: Style.colorBorder);
            enabled: base.enabled;
            wrapMode: TextEdit.WrapAtWordBoundaryOrAnywhere;
            selectByMouse: true;
            selectionColor: Style.colorSelection;
            selectedTextColor: Style.colorEditable;
            activeFocusOnPress: true;
            font {
                family: Style.fontName;
                weight: (Style.useSlimFonts ? Font.Light : Font.Normal);
                pixelSize: Style.fontSizeNormal;
            }
            anchors {
                fill: parent;
                margins: padding;
            }
        }
    }
    TextLabel {
        id: holder;
        color: Style.opacify (Style.colorBorder, 0.85);
        enabled: base.enabled;
        visible: (!input.activeFocus && input.text.trim ().length === 0 && !readOnly);
        horizontalAlignment: input.horizontalAlignment;
        font {
            weight: Font.Normal;
            family: input.font.family;
            pixelSize: input.font.pixelSize;
        }
        anchors {
            top: parent.top;
            left: parent.left;
            right: parent.right;
            margins: padding;
        }
    }
}

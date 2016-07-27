import QtQuick 2.1;
import QtQmlTricks.UiElements 2.0;

FocusScope {
    id: base;
    width: implicitWidth;
    height: implicitHeight;
    implicitWidth: (showButtons ? input.implicitWidth + input.height * 2 : input.implicitWidth);
    implicitHeight: input.implicitHeight;

    property real step        : 1;
    property real value       : 0;
    property real minValue    : 0;
    property real maxValue    : 100;
    property int  decimals    : 0;
    property bool showButtons : true;

    signal edited ();

    function validate () {
        if (enabled) {
            input.apply ();
        }
    }

    TextLabel {
        id: metricsValue;
        color: Style.colorNone;
        text: {
            var txtMin = minValue.toFixed (decimals);
            var txtMax = maxValue.toFixed (decimals);
            var count = Math.max (txtMin.length, txtMax.length);
            return new Array (count +1).join ("0");
        }
    }
    TextButton {
        id: btnDecrease;
        width: (height + Style.roundness);
        icon: SymbolLoader {
            size: Style.fontSizeNormal;
            color: (enabled ? Style.colorForeground : Style.colorBorder);
            symbol: Style.symbolMinus;
        }
        visible: showButtons;
        enabled: (base.enabled && value - step >= minValue);
        autoRepeat: true;
        ExtraAnchors.leftDock: parent;
        onClicked: {
            if (value - step >= minValue) {
                value -= step;
                edited ();
            }
        }
    }
    TextButton {
        id: btnIncrease;
        icon: SymbolLoader {
            size: Style.fontSizeNormal;
            color: (enabled ? Style.colorForeground : Style.colorBorder);
            symbol: Style.symbolPlus;
        }
        width: (height + Style.roundness);
        visible: showButtons;
        enabled: (base.enabled && value + step <= maxValue);
        autoRepeat: true;
        ExtraAnchors.rightDock: parent;
        onClicked: {
            if (value + step <= maxValue) {
                value += step;
                edited ();
            }
        }
    }
    TextBox {
        id: input;
        focus: true;
        enabled: base.enabled;
        rounding: (showButtons ? 0 : Style.roundness);
        hasClear: false;
        textAlign: TextInput.AlignHCenter;
        backColor: (flashEffect ? Style.colorError : Style.colorEditable);
        textColor: (flashEffect ? Style.colorEditable : (hasError ? Style.colorError : Style.colorForeground));
        implicitWidth: (metricsValue.contentWidth + padding * 2);
        validator: RegExpValidator { regExp: (decimals > 0 ? /\d+\.\d+/ : /\d+/); }
        anchors {
            left: (showButtons ? btnDecrease.right : parent.left);
            right: (showButtons ? btnIncrease.left : parent.right);
            leftMargin: (showButtons ? -Style.roundness : 0);
            rightMargin: (showButtons ? -Style.roundness : 0);
        }
        ExtraAnchors.verticalFill: parent;
        onActiveFocusChanged: {
            if (!activeFocus) {
                apply ();
            }
        }
        Keys.onEnterPressed:  {
            event.accepted = false;
            apply ();
        }
        Keys.onReturnPressed: {
            event.accepted = false;
            apply ();
        }
        Keys.onUpPressed:   { btnIncrease.click (); }
        Keys.onDownPressed: { btnDecrease.click (); }

        property bool flashEffect : false;

        readonly property var  number    : parseFloat (text);
        readonly property bool notNumber : isNaN (number);
        readonly property bool tooBig    : (!notNumber ? number > maxValue : false);
        readonly property bool tooSmall  : (!notNumber ? number < minValue : false);
        readonly property bool hasError  : (notNumber || tooBig || tooSmall);

        function apply () {
            if (!notNumber && !tooBig && !tooSmall) {
                base.value = number;
                edited ();
            }
            else {
                text = (base.value.toFixed (decimals));
                animFlash.start ();
            }
        }

        Binding on text { value: (isNaN (base.value) ? "" : base.value.toFixed (decimals)); }
        SequentialAnimation on flashEffect {
            id: animFlash;
            loops: 2;
            running: false;
            alwaysRunToEnd: true;

            PropertyAction { value: true; }
            PauseAnimation { duration: 100; }
            PropertyAction { value: false; }
            PauseAnimation { duration: 100; }
        }
    }
}

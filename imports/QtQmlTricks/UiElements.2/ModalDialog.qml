import QtQuick 2.1;
import QtQmlTricks.UiElements 2.0;

FocusScope {
    id: base;
    anchors.fill: parent;
    Component.onCompleted: {
        var win = Introspector.window (this);
        if (win !== null) {
            priv.previouslyFocusedItem = win.activeFocusItem;
        }
        forceActiveFocus ();
    }

    property string title : "";

    property var message : undefined;

    property int buttons : (buttonOk | buttonCancel);

    property int minWidth : 400;
    property int maxWidth : 600;

    readonly property int buttonOk     : (1 << 0);
    readonly property int buttonYes    : (1 << 1);
    readonly property int buttonNo     : (1 << 2);
    readonly property int buttonCancel : (1 << 3);
    readonly property int buttonAccept : (1 << 4);
    readonly property int buttonReject : (1 << 5);

    default property alias content : container.children;

    function hide () {
        if (priv.previouslyFocusedItem !== null) {
            priv.previouslyFocusedItem.forceActiveFocus ();
        }
        base.destroy ();
    }

    function shake () {
        animShake.start ();
    }

    signal buttonClicked (int buttonType);

    QtObject {
        id: priv;

        property Item previouslyFocusedItem : null;
    }
    MouseArea {
        id: blocker;
        hoverEnabled: Style.useHovering;
        anchors.fill: parent;
        onWheel: { }
        onPressed: { }
        onReleased: { }
    }
    Rectangle {
        id: dimmer;
        color: Style.colorEditable;
        opacity: 0.45;
        anchors.fill: parent;
    }
    Rectangle {
        id: frame;
        color: Style.colorSecondary;
        radius: Style.roundness;
        antialiasing: radius;
        border {
            width: Style.lineSize;
            color: Style.colorSelection;
        }
        anchors {
            fill: layout;
            margins: -Style.spacingBig;
        }
    }
    StretchColumnContainer {
        id: layout;
        spacing: Style.spacingBig;
        anchors.centerIn: parent;

        SequentialAnimation on anchors.horizontalCenterOffset {
            id: animShake;
            loops: 2;
            running: false;
            alwaysRunToEnd: true;

            PropertyAnimation {
                to: 30;
                duration: 40;
            }
            PropertyAnimation {
                to: -30;
                duration: 40;
            }
            PropertyAnimation {
                to: 0;
                duration: 40;
            }
        }
        TextLabel {
            id: lblTitle;
            text: base.title;
            visible: (text.trim () !== "");
            font.pixelSize: Style.fontSizeTitle;
        }
        Line { visible: lblTitle.visible; }
        Stretcher {
            visible: lblMsg.visible;
            implicitHeight: lblMsg.contentHeight;
            implicitWidth: Math.max (Math.min (lblMsg.contentWidth, maxWidth), minWidth);

            TextLabel {
                id: lblMsg;
                text: {
                    var ret = "";
                    if (base.message !== undefined) {
                        if (Array.isArray (base.message)) {
                            ret = base.message.join ("\n");
                        }
                        else {
                            ret = base.message.toString ();
                        }
                    }
                    return ret;
                }
                visible: (text.trim () !== "");
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere;
                horizontalAlignment: Text.AlignJustify;
                ExtraAnchors.horizontalFill: parent;
            }
        }
        Stretcher {
            visible: (container.children.length > 0);
            implicitWidth: Math.max (Math.min (container.implicitWidth, maxWidth), minWidth);
            implicitHeight: container.implicitHeight;

            StretchColumnContainer {
                id: container;
                spacing: Style.spacingBig;
                ExtraAnchors.topDock: parent;
            }
        }
        Line { }
        StretchRowContainer {
            id: row;
            spacing: Style.spacingNormal;

            Stretcher { }
            GridContainer {
                cols: capacity;
                capacity: repeater.entries.length;
                colSpacing: Style.spacingNormal;

                Repeater {
                    id: repeater;
                    model: entries;
                    delegate: TextButton {
                        text: (modelData ["label"] || "");
                        icon: SymbolLoader {
                            size: Style.fontSizeNormal;
                            color: Style.colorForeground;
                            symbol: (modelData ["symbol"] || null);
                        }
                        onClicked: { base.buttonClicked (modelData ["type"] || 0); }
                    }

                    readonly property var entries : {
                        var ret = [];
                        var tmp = [
                                    { "type" : buttonCancel, "label" : qsTr ("Cancel"), "symbol" : Style.symbolCross },
                                    { "type" : buttonNo,     "label" : qsTr ("No"),     "symbol" : Style.symbolCross },
                                    { "type" : buttonReject, "label" : qsTr ("Reject"), "symbol" : Style.symbolCross },
                                    { "type" : buttonAccept, "label" : qsTr ("Accept"), "symbol" : Style.symbolCheck },
                                    { "type" : buttonYes,    "label" : qsTr ("Yes"),    "symbol" : Style.symbolCheck },
                                    { "type" : buttonOk,     "label" : qsTr ("Ok"),     "symbol" : Style.symbolCheck },
                                ];
                        tmp.forEach (function (item) {
                            if (buttons & (item ["type"] || 0)) {
                                ret.push (item);
                            }
                        });
                        return ret;
                    }
                }
            }
            Stretcher { }
        }
    }
}

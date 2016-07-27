import QtQuick 2.1;
import QtQmlTricks.UiElements 2.0;

Item {
    id: accordion;

    property alias background : rect.color;

    property Group currentTab : null;

    default property alias content : container.children;

    readonly property var tabs : {
        var ret = [];
        for (var idx = 0; idx < content.length; idx++) {
            var item = content [idx];
            if (Introspector.inherits (item, testGroup)) {
                ret.push (item);
            }
        }
        return ret;
    }

    readonly property int tabSize  : (Style.spacingBig * 2);
    readonly property int paneSize : (height - tabs.length * (tabSize + Style.lineSize));

    Group { id: testGroup; }
    Rectangle {
        id: rect;
        color: Style.colorSecondary;
        anchors.fill: parent;
    }
    Item {
        id: container;
        height: paneSize;
        anchors.topMargin: ((tabSize + Style.lineSize) * (tabs.indexOf (currentTab) +1));
        ExtraAnchors.topDock: parent;

        // NOTE : tabs content here
    }
    Column {
        anchors.fill: parent;

        Repeater {
            model: tabs;
            delegate: Column {
                id: col;
                states: [
                    State {
                        name: "text_and_icon";
                        when: (col.group.icon !== null);

                        AnchorChanges {
                            target: lbl;
                            anchors {
                                left: ico.right;
                                verticalCenter: parent.verticalCenter;
                            }
                        }
                        AnchorChanges {
                            target: ico;
                            anchors {
                                left: parent.left;
                                verticalCenter: parent.verticalCenter;
                            }
                        }
                    },
                    State {
                        name: "text_only";
                        when: (col.group.icon === null);

                        AnchorChanges {
                            target: lbl;
                            anchors {
                                verticalCenter: parent.verticalCenter;
                                horizontalCenter: parent.horizontalCenter;
                            }
                        }
                    }
                ]
                ExtraAnchors.horizontalFill: parent;

                readonly property Group group : modelData;

                MouseArea {
                    height: tabSize;
                    ExtraAnchors.horizontalFill: parent;
                    onClicked: { currentTab = (currentTab !== col.group ? col.group : null); }

                    Rectangle {
                        gradient: (col.group === currentTab
                                   ? Style.gradientShaded (Style.colorHighlight, Style.colorSecondary)
                                   : (parent.pressed
                                      ? Style.gradientPressed ()
                                      : Style.gradientIdle ()));
                        anchors.fill: parent;
                    }
                    Loader {
                        id: ico;
                        enabled: col.enabled;
                        sourceComponent: col.group.icon;
                        anchors.margins: Style.spacingNormal;
                    }
                    TextLabel {
                        id: lbl;
                        text: col.group.title;
                        anchors.margins: Style.spacingNormal;
                    }
                }
                Binding {
                    target: col.group ["anchors"];
                    property: "fill";
                    value: container;
                }
                Binding {
                    target: col.group;
                    property: "visible";
                    value: (col.group === currentTab);
                }
                Stretcher {
                    id: placeholder;
                    height: paneSize;
                    visible: (col.group === currentTab);
                    ExtraAnchors.horizontalFill: parent;
                }
                Line { ExtraAnchors.horizontalFill: parent; }
            }
        }
    }
}

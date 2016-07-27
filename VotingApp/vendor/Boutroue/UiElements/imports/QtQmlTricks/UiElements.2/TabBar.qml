import QtQuick 2.1;
import QtQmlTricks.UiElements 2.0;

Item {
    id: base;

    property alias background : rect.color;

    property int tabsSize : (Style.spacingBig * 2);

    property int extraPaddingBeforeTabs : 0;
    property int extraPaddingAfterTabs  : 0;

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

    Group { id: testGroup; }
    Rectangle {
        id: rect;
        color: Style.colorSecondary;
        anchors.bottom: bar.bottom;
        ExtraAnchors.topDock: parent;
    }
    Line {
        anchors.bottom: bar.bottom;
        ExtraAnchors.horizontalFill: parent;
    }
    GridContainer {
        id: bar;
        clip: true;
        cols: capacity;
        capacity: tabs.length;
        colSpacing: Style.spacingSmall;
        anchors {
            topMargin: bar.colSpacing;
            leftMargin: (bar.colSpacing + extraPaddingBeforeTabs);
            rightMargin: (bar.colSpacing + extraPaddingAfterTabs);
        }
        ExtraAnchors.topDock: parent;

        Repeater {
            model: tabs;
            delegate: MouseArea {
                id: clicker;
                implicitHeight: tabsSize;
                states: [
                    State {
                        name: "text_and_icon";
                        when: (clicker.group.icon !== null && clicker.group.title !== "");

                        AnchorChanges {
                            target: lbl;
                            anchors {
                                left: ico.right;
                                right: parent.right;
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
                        PropertyChanges {
                            target: lbl;
                            visible: true;
                            horizontalAlignment: Text.AlignLeft;
                        }
                        PropertyChanges {
                            target: ico;
                            visible: true;
                        }
                    },
                    State {
                        name: "text_only";
                        when: (clicker.group.icon === null && clicker.group.title !== "");

                        AnchorChanges {
                            target: lbl;
                            anchors {
                                left: parent.left;
                                right: parent.right;
                                verticalCenter: parent.verticalCenter;
                            }
                        }
                        PropertyChanges {
                            target: lbl;
                            visible: true;
                            horizontalAlignment: Text.AlignHCenter;
                        }
                        PropertyChanges {
                            target: ico;
                            visible: false;
                        }
                    },
                    State {
                        name: "icon_only";
                        when: (clicker.group.icon !== null && clicker.group.title === "");

                        AnchorChanges {
                            target: ico;
                            anchors {
                                verticalCenter: parent.verticalCenter;
                                horizontalCenter: parent.horizontalCenter;
                            }
                        }
                        PropertyChanges {
                            target: lbl;
                            visible: false;
                        }
                        PropertyChanges {
                            target: ico;
                            visible: true;
                        }
                    }
                ]
                onClicked: { currentTab = modelData; }

                readonly property Group group : modelData;

                Rectangle {
                    color: Style.colorNone;
                    radius: Style.roundness;
                    gradient: (clicker.pressed
                               ? Style.gradientPressed ()
                               : (currentTab === clicker.group
                                  ? Style.gradientShaded ()
                                  : null));
                    antialiasing: radius;
                    border {
                        width: Style.lineSize;
                        color: Style.colorBorder;
                    }
                    states: State {
                        when: (clicker.group !== null);

                        PropertyChanges {
                            target: clicker.group;
                            visible: (currentTab === clicker.group);
                            anchors.fill: container;
                        }
                    }
                    anchors {
                        fill: parent;
                        bottomMargin: -radius;
                    }
                }
                Loader {
                    id: ico;
                    enabled: clicker.enabled;
                    sourceComponent: clicker.group.icon;
                    anchors.margins: Style.spacingNormal;
                }
                TextLabel {
                    id: lbl;
                    text: clicker.group.title;
                    clip: (contentWidth > width);
                    anchors.margins: Style.spacingNormal;
                }
                Line {
                    visible: (currentTab !== clicker.group);
                    ExtraAnchors.bottomDock: parent;
                }
            }
        }
    }
    FocusScope {
        id: container;
        anchors.top: bar.bottom;
        ExtraAnchors.bottomDock: parent;

        // NOTE : CONTENT HERE
    }
}

import QtQuick 2.1;
import QtQmlTricks.UiElements 2.0;

FocusScope {
    id: base;

    property string folder     : FileSystem.homePath;
    property string rootFolder : FileSystem.rootPath;

    property bool showFiles  : true;
    property bool showHidden : false;

    property var nameFilters : [];

    property var iconProvider  : (function (entry) { return mimeHelper.getSvgIconPathForUrl (entry.url); });
    property var labelProvider : (function (entry) { return entry.name + (entry.isDir ? "/" : ""); });

    property int selectionType : (selectFile);

    readonly property int selectFile     : 1;
    readonly property int selectDir      : 2;
    readonly property int selectAllowNew : 4;

    readonly property var entries : FileSystem.list (folder, nameFilters, showHidden, showFiles);

    readonly property string currentPath : (inputName.value !== "" ? (folder + "/" + inputName.value) : "");

    function select (name) {
        var tmp = (name || "").trim ();
        inputName.text = tmp;
    }

    function goToFolder (path) {
        var tmp = (path || "").trim ();
        if (tmp !== "" && FileSystem.exists (tmp)) {
            folder = tmp;
        }
    }

    MimeIconsHelper { id: mimeHelper; }
    StretchColumnContainer {
        spacing: Style.spacingNormal;
        anchors.fill: parent;

        StretchRowContainer {
            spacing: Style.spacingNormal;
            ExtraAnchors.horizontalFill: parent;

            ComboList {
                model: FileSystem.drivesList;
                visible: (FileSystem.rootPath !== "/");
                delegate: ComboListDelegateForSimpleVar { }
                anchors.verticalCenter: parent.verticalCenter;
                onCurrentKeyChanged: {
                    if (currentKey !== undefined && currentKey !== "" && ready) {
                        rootFolder = currentKey;
                        goToFolder (rootFolder);
                    }
                }
                Component.onCompleted: {
                    selectByKey (FileSystem.rootPath);
                    ready = true;
                }

                property bool ready : false;
            }
            Stretcher {
                height: implicitHeight;
                implicitHeight: path.height;
                anchors.verticalCenter: parent.verticalCenter;

                TextLabel {
                    id: path;
                    text: folder;
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere;
                    font.pixelSize: Style.fontSizeSmall;
                    anchors.verticalCenter: parent.verticalCenter;
                    ExtraAnchors.horizontalFill: parent;
                }
            }
            TextButton {
                text: qsTr ("Parent");
                enabled: (folder !== rootFolder);
                icon: SvgIconLoader {
                    icon: "actions/chevron-up";
                    size: Style.iconSize (1);
                    color: Style.colorForeground;
                }
                anchors.verticalCenter: parent.verticalCenter;
                onClicked: { goToFolder (FileSystem.parentDir (folder)); }
            }
        }
        ScrollContainer {
            placeholder: (list.count === 0 ? qsTr ("Empty.") : "");

            ListView {
                id: list;
                model: entries;
                delegate: MouseArea {
                    id: delegate;
                    height: (Math.max (label.height, img.height) + label.anchors.margins * 2);
                    ExtraAnchors.horizontalFill: parent;
                    onClicked: {
                        if (entry.isDir) {
                            select ((selectionType & selectDir) ? entry.name : "");
                        }
                        else if (entry.isFile) {
                            select ((selectionType & selectFile) ? entry.name : "");
                        }
                        else { }
                    }
                    onDoubleClicked: {
                        if (entry.isDir) {
                            select ("");
                            goToFolder (entry.path);
                        }
                        else { }
                    }

                    readonly property FileSystemModelEntry entry : modelData;

                    readonly property bool isCurrent : (entry.path === currentPath);

                    Rectangle {
                        color: Style.colorHighlight;
                        opacity: 0.35;
                        visible: delegate.isCurrent;
                        anchors.fill: parent;
                    }
                    Line {
                        opacity: 0.65;
                        ExtraAnchors.bottomDock: parent;
                    }
                    SvgIconLoader {
                        id: img;
                        size: Style.realPixels (24);
                        icon: iconProvider (delegate.entry);
                        anchors {
                            left: parent.left;
                            margins: Style.spacingNormal;
                            verticalCenter: parent.verticalCenter;
                        }
                    }
                    TextLabel {
                        id: label;
                        text: labelProvider (delegate.entry);
                        elide: Text.ElideRight;
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere;
                        maximumLineCount: 3;
                        anchors {
                            left: img.right;
                            right: parent.right;
                            margins: Style.spacingNormal;
                            verticalCenter: parent.verticalCenter;
                        }
                    }
                }
            }
        }
        StretchRowContainer {
            spacing: Style.spacingNormal;

            TextLabel {
                text: qsTr ("Name :");
                anchors.verticalCenter: parent.verticalCenter;
            }
            TextBox {
                id: inputName;
                focus: true;
                enabled: (selectionType & selectAllowNew);
                implicitWidth: -1;
                anchors.verticalCenter: parent.verticalCenter;

                readonly property string value : (text.trim ());
            }
        }
    }
}

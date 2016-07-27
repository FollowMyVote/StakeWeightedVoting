import QtQuick 2.1;

Item {
    id: base;

    property int    index     : -1;
    property var    key       : undefined;
    property string value     : "";
    property bool   active    : false;

    // NOTE : automatically set by the Loader, proxying to model/modelData
    property var    model     : undefined;
    property var    modelData : undefined;
}

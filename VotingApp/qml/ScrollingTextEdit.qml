import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQml 2.2

import VPlayApps 1.0

import FollowMyVote.StakeWeightedVoting 1.0

FocusScope {
    property alias textEdit: textEdit
    property alias text: textEdit.text

    Flickable {
        id: scroller
        anchors.fill: parent
        contentWidth: textEdit.paintedWidth
        contentHeight: contestDescription.paintedHeight
        clip: true

        function ensureVisible(r)
        {
            if (contentX >= r.x)
                contentX = r.x;
            else if (contentX+width <= r.x+r.width)
                contentX = r.x+r.width-width;
            if (contentY >= r.y)
                contentY = r.y;
            else if (contentY+height <= r.y+r.height)
                contentY = r.y+r.height-height;
        }

        AppTextEdit {
            id: textEdit
            width: scroller.width
            height: scroller.height
            wrapMode: TextEdit.Wrap
            onCursorPositionChanged: scroller.ensureVisible(cursorRectangle)
            focus: true
        }
    }
}

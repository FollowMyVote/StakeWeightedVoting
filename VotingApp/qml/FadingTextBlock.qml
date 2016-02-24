import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0
import QtGraphicalEffects 1.0

import VPlayApps 1.0

import FollowMyVote.StakeWeightedVoting 1.0

/**
 * The FadingTextBlock is an AppText element which has a maximum height. If the text is higher than the maximum height,
 * the text will fade vertically to transparent, indicating to the user that some of the text is hidden.
 */
Item {
    implicitHeight: truncated? maximumHeight : textDisplay.implicitHeight

    // The maximum height available to show text
    property int maximumHeight
    // The text to display
    property alias text: textDisplay.text
    // Direct access to the AppText item
    property alias textItem: textDisplay

    // Output: True if the text is larger than the allowed space; false otherwise
    property bool truncated: textDisplay.implicitHeight > maximumHeight

    AppText {
        id: textDisplay
        visible: !parent.truncated
        anchors.fill: parent
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        verticalAlignment: Text.AlignTop
    }
    Rectangle {
        id: fadeGradient
        visible: false
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.5; color: "black" }
            GradientStop { position: 1; color: "transparent" }
        }
    }
    OpacityMask {
        visible: truncated
        anchors.fill: parent
        maskSource: fadeGradient
        source: textDisplay
    }
}

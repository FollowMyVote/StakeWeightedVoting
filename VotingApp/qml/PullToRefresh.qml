/*
 * Copyright 2015 Follow My Vote, Inc.
 * This file is part of The Follow My Vote Stake-Weighted Voting Application ("SWV").
 * 
 * SWV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * SWV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with SWV.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.3
import QtQuick.Layouts 1.1

import Material 0.1

ColumnLayout {
    id: pullToRefresh
    y: Math.min(-height - view.contentY - spacing, spacing)
    anchors.horizontalCenter: parent.horizontalCenter
    spacing: Units.dp(10)
    height: Units.dp(100)

    property Flickable view
    property alias text: pullToRefreshLabel.text
    readonly property bool fullyPulled: pullToRefreshIndicator.value === 1

    signal triggered

    Connections {
        target: view
        onDragEnded: if( fullyPulled ) pullToRefresh.triggered()
    }

    Label {
        id: pullToRefreshLabel
        style: "headline"
        Layout.alignment: Qt.AlignHCenter
    }
    ProgressCircle {
        id: pullToRefreshIndicator
        Layout.alignment: Qt.AlignHCenter
        Layout.fillHeight: true
        width: height
        indeterminate: false
        value: Math.max(0, Math.min(1, (view.contentY + pullToRefreshLabel.height)
                                    / -pullToRefresh.height))
    }
}

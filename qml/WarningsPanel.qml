pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    property var warningModel: null

    signal warningClicked(var polygonPath)

    NumberAnimation {
        id: scrollAnimation
        target: warningScrollView.contentItem
        property: "contentY"
        duration: 300
        easing.type: Easing.OutCubic
    }

    function scrollToWarning(warningIndex) {
        // Get the warning item and calculate its Y position
        var itemY = 0;
        for (var i = 0; i < warningIndex; i++) {
            var item = warningRepeater.itemAt(i);
            if (item)
                itemY += item.height + warningColumn.spacing;
        }
        // Animate to target position
        scrollAnimation.to = itemY;
        scrollAnimation.restart();
    }

    color: "#2b2b2b"

    Column {
        anchors.fill: parent
        anchors.leftMargin: 8
        spacing: 8

        // Flood Warnings Section
        Text {
            text: "Flood Warnings"
            color: "white"
            font.pixelSize: 16
            font.bold: true
        }

        ScrollView {
            id: warningScrollView
            width: parent.width
            height: parent.height - y - 30
            clip: true
            contentWidth: width
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            Column {
                id: warningColumn

                width: parent.width - 25
                spacing: 12

                Repeater {
                    id: warningRepeater

                    model: root.warningModel

                    delegate: Rectangle {
                        id: warningItem
                        required property var model

                        property bool expanded: false
                        property bool showExpandButton: warningItem.model.message && warningItem.model.message !== ""

                        width: parent.width
                        anchors.horizontalCenter: parent.horizontalCenter
                        color: "#3a3a3a"
                        radius: 6
                        border.color: {
                            if (warningItem.model.severityLevel === 1)
                                return "#ff4444";

                            if (warningItem.model.severityLevel === 2)
                                return "#ff9944";

                            if (warningItem.model.severityLevel === 3)
                                return "#ffdd44";

                            return "#888888";
                        }
                        border.width: 2
                        // Calculate height based on content and expanded state
                        height: {
                            var contentHeight = warningItemColumn.height + 16;
                            if (showExpandButton)
                                return contentHeight + expandButton.height + 8;

                            return contentHeight;
                        }

                        Rectangle {
                            id: expandButtonArea
                            anchors.fill: parent
                            visible: false
                        }

                        // Main clickable area for map navigation
                        MouseArea {
                            id: mapNavigationMouseArea

                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            hoverEnabled: true
                            onClicked: mouse => {
                                // Only trigger map navigation if we didn't click on the expand button
                                if (!expandButtonMouseArea.containsMouse) {
                                    if (warningItem.model.polygonPath && warningItem.model.polygonPath.length > 0)
                                        root.warningClicked(warningItem.model.polygonPath);
                                }
                            }
                        }

                        Column {
                            id: warningItemColumn

                            anchors.margins: 8
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: parent.top
                            spacing: 6

                            Text {
                                text: warningItem.model.description || "Flood Warning"
                                color: "white"
                                font.pixelSize: 14
                                font.bold: true
                                wrapMode: Text.WordWrap
                                width: parent.width
                            }

                            Text {
                                text: "Severity: " + (warningItem.model.severity || "Unknown")
                                color: "#cccccc"
                                font.pixelSize: 12
                                wrapMode: Text.WordWrap
                                width: parent.width
                            }

                            Text {
                                text: "Area: " + (warningItem.model.eaAreaName || "Unknown")
                                color: "#cccccc"
                                font.pixelSize: 12
                                wrapMode: Text.WordWrap
                                width: parent.width
                            }

                            Text {
                                id: messageText

                                text: warningItem.model.message || ""
                                color: "#aaaaaa"
                                font.pixelSize: 11
                                wrapMode: Text.WordWrap
                                width: parent.width
                                visible: warningItem.expanded && warningItem.model.message !== ""
                            }
                        }

                        // Expand/Collapse button
                        Rectangle {
                            id: expandButton

                            width: 80
                            height: 24
                            radius: 4
                            color: "#4a4a4a"
                            visible: warningItem.showExpandButton

                            anchors {
                                bottom: parent.bottom
                                horizontalCenter: parent.horizontalCenter
                                bottomMargin: 8
                            }

                            // Handles the expand/collapse functionality
                            MouseArea {
                                id: expandButtonMouseArea

                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                hoverEnabled: true
                                onClicked: mouse => {
                                    warningItem.expanded = !warningItem.expanded;
                                    // Prevent this click from propagating to the parent MouseArea
                                    mouse.accepted = true;
                                }
                            }

                            Text {
                                anchors.centerIn: parent
                                text: warningItem.expanded ? "▲" : "▼"
                                color: "white"
                                font.pixelSize: 12
                                font.bold: true
                            }
                        }
                    }
                }
            }
        }

        Text {
            text: "Panel size: " + root.width + " × " + root.height
            color: "#888888"
            font.pixelSize: 11
        }
    }
}

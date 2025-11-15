pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    property var stationModel: null
    property var floodWarningModel: null
    property var selectedStation: null
    property var stationMeasures: []

    signal floodWarningClicked(var polygonPath)
    signal stationClosed

    function scrollToWarning(warningIndex) {
        // Get the warning item and calculate its Y position
        var itemY = 0;
        for (var i = 0; i < warningIndex; i++) {
            var item = warningRepeater.itemAt(i);
            if (item)
                itemY += item.height + warningColumn.spacing;
        }
        // Set contentY to position the item at the top
        warningScrollView.contentItem.contentY = itemY;
    }

    color: "#2b2b2b"
    border.color: "white"
    border.width: 1
    onSelectedStationChanged: {
        if (selectedStation) {
            // Fetch measures for the selected station
            root.stationModel.fetchMeasures(selectedStation.index);
            root.stationMeasures = root.stationModel.getMeasures(selectedStation.index);
        } else {
            root.stationMeasures = [];
        }
    }

    Column {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 8

        Row {
            width: parent.width
            spacing: 8

            Text {
                text: root.selectedStation ? root.selectedStation.data.label : "No station selected"
                color: "white"
                font.pixelSize: 18
                font.bold: true
                elide: Text.ElideRight
                width: parent.width - closeBtn.width - 8
            }

            Rectangle {
                id: closeBtn

                width: 28
                height: 28
                radius: 4
                color: "#555555"
                anchors.verticalCenter: parent.verticalCenter
                visible: root.selectedStation !== null

                Text {
                    anchors.centerIn: parent
                    text: "✕"
                    color: "white"
                    font.pixelSize: 14
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: mouse => {
                        root.stationClosed();
                    }
                }
            }
        }

        Text {
            text: root.selectedStation ? root.selectedStation.data.town : "Tap a marker on the map to see details."
            color: "#cccccc"
            font.pixelSize: 14
            wrapMode: Text.WordWrap
            width: parent.width
        }

        // Details group - only visible when something is selected
        Column {
            spacing: 6
            visible: root.selectedStation !== null
            width: parent.width

            Row {
                spacing: 8

                Text {
                    text: "RLOI ID:"
                    color: "#aaaaaa"
                    font.pixelSize: 13
                }

                Text {
                    text: root.selectedStation ? root.selectedStation.data.RLOIid : ""
                    color: "white"
                    font.pixelSize: 13
                }
            }

            Row {
                spacing: 8

                Text {
                    text: "River:"
                    color: "#aaaaaa"
                    font.pixelSize: 13
                }

                Text {
                    text: root.selectedStation ? root.selectedStation.data.riverName : ""
                    color: "white"
                    font.pixelSize: 13
                }
            }

            Row {
                spacing: 8

                Text {
                    text: "Catchment:"
                    color: "#aaaaaa"
                    font.pixelSize: 13
                }

                Text {
                    text: root.selectedStation ? root.selectedStation.data.catchmentName : ""
                    color: "white"
                    font.pixelSize: 13
                }
            }

            Row {
                spacing: 8

                Text {
                    text: "Opened:"
                    color: "#aaaaaa"
                    font.pixelSize: 13
                }

                Text {
                    text: root.selectedStation ? root.selectedStation.data.dateOpened : ""
                    color: "white"
                    font.pixelSize: 13
                }
            }

            Row {
                spacing: 8

                Text {
                    text: "Lat:"
                    color: "#aaaaaa"
                    font.pixelSize: 13
                }

                Text {
                    text: root.selectedStation ? root.selectedStation.data.latitude : ""
                    color: "white"
                    font.pixelSize: 13
                }
            }

            Row {
                spacing: 8

                Text {
                    text: "Lon:"
                    color: "#aaaaaa"
                    font.pixelSize: 13
                }

                Text {
                    text: root.selectedStation ? root.selectedStation.data.longitude : ""
                    color: "white"
                    font.pixelSize: 13
                }
            }
        }

        // Measurements Section
        Column {
            spacing: 6
            visible: root.selectedStation !== null && root.stationMeasures.length > 0
            width: parent.width

            Rectangle {
                width: parent.width
                height: 1
                color: "#444444"
            }

            Text {
                text: "Measurements"
                color: "white"
                font.pixelSize: 14
                font.bold: true
            }

            Repeater {
                model: root.stationMeasures

                delegate: Column {
                    id: measurementDelegate
                    required property var modelData

                    width: parent.width
                    spacing: 2

                    Row {
                        spacing: 8
                        width: parent.width

                        Text {
                            text: measurementDelegate.modelData.parameterName
                            color: "#88ccff"
                            font.pixelSize: 13
                            font.bold: true
                        }

                        Text {
                            text: measurementDelegate.modelData.qualifier ? "(" + measurementDelegate.modelData.qualifier + ")" : ""
                            color: "#aaaaaa"
                            font.pixelSize: 11
                            visible: measurementDelegate.modelData.qualifier !== ""
                        }
                    }

                    Row {
                        spacing: 8

                        Text {
                            text: "Latest:"
                            color: "#aaaaaa"
                            font.pixelSize: 12
                        }

                        Text {
                            text: measurementDelegate.modelData.latestReading !== undefined && measurementDelegate.modelData.latestReading !== null ? measurementDelegate.modelData.latestReading.toFixed(2) + " " + measurementDelegate.modelData.unitName : "No data"
                            color: "white"
                            font.pixelSize: 12
                        }
                    }

                    Rectangle {
                        width: parent.width
                        height: 1
                        color: "#333333"
                    }
                }
            }
        }

        Rectangle {
            width: parent.width
            height: 1
            color: "#444444"
        }

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
            height: parent.parent.height - y - 30
            clip: true
            contentWidth: width
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            Column {
                id: warningColumn

                width: parent.parent.width
                spacing: 12

                Repeater {
                    id: warningRepeater

                    model: root.floodWarningModel

                    delegate: Rectangle {
                        id: warningItem
                        required property var model

                        property bool expanded: false
                        property bool showExpandButton: warningItem.model.message && warningItem.model.message !== ""

                        width: parent.width - 16
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
                                        root.floodWarningClicked(warningItem.model.polygonPath);
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

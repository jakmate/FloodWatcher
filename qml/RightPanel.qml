import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property var selectedStation: null
    property var stationMeasures: []
    signal floodWarningClicked(var polygonPath)
    signal stationClosed()
    
    color: "#2b2b2b"
    border.color: "white"
    border.width: 1

    onSelectedStationChanged: {
        if (selectedStation) {
            // Fetch measures for the selected station
            stationModel.fetchMeasures(selectedStation.index)
            stationMeasures = stationModel.getMeasures(selectedStation.index)
        } else {
            stationMeasures = []
        }
    }

    function scrollToWarning(warningIndex) {
        // Get the warning item and calculate its Y position
        var itemY = 0
        for (var i = 0; i < warningIndex; i++) {
            var item = warningRepeater.itemAt(i)
            if (item) {
                itemY += item.height + warningColumn.spacing
            }
        }
        
        // Set contentY to position the item at the top
        warningScrollView.contentItem.contentY = itemY
    }

    Column {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 8

        Row {
            width: parent.width
            spacing: 8
            Text {
                text: selectedStation ? selectedStation.data.label : "No station selected"
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
                visible: selectedStation !== null
                Text {
                    anchors.centerIn: parent
                    text: "✕"
                    color: "white"
                    font.pixelSize: 14
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        root.stationClosed()
                    }
                }
            }
        }

        Text {
            text: selectedStation ? selectedStation.data.town : "Tap a marker on the map to see details."
            color: "#cccccc"
            font.pixelSize: 14
            wrapMode: Text.WordWrap
            width: parent.width
        }

        // Details group - only visible when something is selected
        Column {
            spacing: 6
            visible: selectedStation !== null
            width: parent.width

            Row { spacing: 8
                Text { text: "RLOI ID:"; color: "#aaaaaa"; font.pixelSize: 13 }
                Text { text: selectedStation ? selectedStation.data.RLOIid : ""; color: "white"; font.pixelSize: 13 }
            }
            Row { spacing: 8
                Text { text: "River:"; color: "#aaaaaa"; font.pixelSize: 13 }
                Text { text: selectedStation ? selectedStation.data.riverName : ""; color: "white"; font.pixelSize: 13 }
            }
            Row { spacing: 8
                Text { text: "Catchment:"; color: "#aaaaaa"; font.pixelSize: 13 }
                Text { text: selectedStation ? selectedStation.data.catchmentName : ""; color: "white"; font.pixelSize: 13 }
            }
            Row { spacing: 8
                Text { text: "Opened:"; color: "#aaaaaa"; font.pixelSize: 13 }
                Text { text: selectedStation ? selectedStation.data.dateOpened : ""; color: "white"; font.pixelSize: 13 }
            }
            Row { spacing: 8
                Text { text: "Lat:"; color: "#aaaaaa"; font.pixelSize: 13 }
                Text { text: selectedStation ? selectedStation.data.latitude : ""; color: "white"; font.pixelSize: 13 }
            }
            Row { spacing: 8
                Text { text: "Lon:"; color: "#aaaaaa"; font.pixelSize: 13 }
                Text { text: selectedStation ? selectedStation.data.longitude : ""; color: "white"; font.pixelSize: 13 }
            }
        }

        // Measurements Section
        Column {
            spacing: 6
            visible: selectedStation !== null && stationMeasures.length > 0
            width: parent.width

            Rectangle { width: parent.width; height: 1; color: "#444444" }

            Text {
                text: "Measurements"
                color: "white"
                font.pixelSize: 14
                font.bold: true
            }

            Repeater {
                model: stationMeasures
                
                Column {
                    width: parent.width
                    spacing: 2
                    
                    Row {
                        spacing: 8
                        width: parent.width
                        
                        Text {
                            text: modelData.parameterName
                            color: "#88ccff"
                            font.pixelSize: 13
                            font.bold: true
                        }
                        
                        Text {
                            text: modelData.qualifier ? "(" + modelData.qualifier + ")" : ""
                            color: "#aaaaaa"
                            font.pixelSize: 11
                            visible: modelData.qualifier !== ""
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
                            text: modelData.latestReading !== undefined && modelData.latestReading !== null
                                  ? modelData.latestReading.toFixed(2) + " " + modelData.unitName
                                  : "No data"
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

        Rectangle { width: parent.width; height: 1; color: "#444444" }

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
                    model: floodWarningModel

                    Rectangle {
                        id: warningItem
                        width: parent.width - 16
                        height: warningItemColumn.height + 16
                        color: "#3a3a3a"
                        radius: 6
                        border.color: {
                            if (model.severityLevel === 1) return "#ff4444"
                            if (model.severityLevel === 2) return "#ff9944"
                            if (model.severityLevel === 3) return "#ffdd44"
                            return "#888888"
                        }
                        border.width: 2

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (model.polygonPath && model.polygonPath.length > 0) {
                                    root.floodWarningClicked(model.polygonPath)
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
                                text: model.description || "Flood Warning"
                                color: "white"
                                font.pixelSize: 14
                                font.bold: true
                                wrapMode: Text.WordWrap
                                width: parent.width
                            }

                            Text {
                                text: "Severity: " + (model.severity || "Unknown")
                                color: "#cccccc"
                                font.pixelSize: 12
                                wrapMode: Text.WordWrap
                                width: parent.width
                            }

                            Text {
                                text: "Area: " + (model.eaAreaName || "Unknown")
                                color: "#cccccc"
                                font.pixelSize: 12
                                wrapMode: Text.WordWrap
                                width: parent.width
                            }

                            Text {
                                text: model.message || ""
                                color: "#aaaaaa"
                                font.pixelSize: 11
                                wrapMode: Text.WordWrap
                                width: parent.width
                                visible: model.message !== ""
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
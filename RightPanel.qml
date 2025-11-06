import QtQuick 2.15

Rectangle {
    id: root
    property var selectedStation: null
    color: "#2b2b2b"
    border.color: "white"
    border.width: 1

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
            }

            Rectangle {
                id: closeBtn; width: 28; height: 28; radius: 4; color: "#555555"
                anchors.verticalCenter: parent.verticalCenter
                Text { anchors.centerIn: parent; text: "✕"; color: "white"; font.pixelSize: 14 }
                MouseArea { anchors.fill: parent; onClicked: root.selectedStation = null }
            }
        }

        Text {
            text: selectedStation ? selectedStation.data.town : "Tap a marker on the map to see details."
            color: "#cccccc"
            font.pixelSize: 14
            wrapMode: Text.WordWrap
        }

        // Details group - only visible when something is selected
        Column {
            spacing: 6
            visible: selectedStation !== null

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

        Rectangle { width: parent.width - 32; height: 1; color: "#444444" }
        Text { text: "Panel size: " + root.width + " × " + root.height; color: "#888888"; font.pixelSize: 11 }
    }
}

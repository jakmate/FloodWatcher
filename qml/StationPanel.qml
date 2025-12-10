pragma ComponentBehavior: Bound
import QtQuick

Item {
    id: root

    property var stationModel: null
    property var selectedStation: null
    readonly property var stationMeasures: selectedStation ? (selectedStation.model.measures || []) : []

    signal stationClosed

    implicitHeight: contentColumn.implicitHeight + 32

    onSelectedStationChanged: {
        if (selectedStation) {
            root.stationModel.fetchMeasures(selectedStation.index);
        }
    }

    Column {
        id: contentColumn
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: 8
        }
        spacing: 8

        Row {
            width: parent.width
            spacing: 8

            Text {
                text: root.selectedStation ? root.selectedStation.model.label : "No station selected"
                color: "white"
                font.pixelSize: 18
                font.bold: true
                elide: Text.ElideRight
                width: parent.width - closeBtn.width - 8
                wrapMode: Text.WordWrap
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
            text: root.selectedStation ? root.selectedStation.model.town : "Tap a marker on the map to see details."
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
                    text: root.selectedStation ? root.selectedStation.model.RLOIid : ""
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
                    text: root.selectedStation ? root.selectedStation.model.riverName : ""
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
                    text: root.selectedStation ? root.selectedStation.model.catchmentName : ""
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
                    text: root.selectedStation ? root.selectedStation.model.dateOpened : ""
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
                    text: root.selectedStation ? root.selectedStation.model.latitude : ""
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
                    text: root.selectedStation ? root.selectedStation.model.longitude : ""
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

        Text {
            text: "Panel size: " + root.width + " × " + root.height
            color: "#888888"
            font.pixelSize: 11
        }
    }
}

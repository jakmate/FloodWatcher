import QtQuick 2.15
import QtLocation 5.15
import QtPositioning 5.15

Window {
    id: root
    width: 800 //Screen.width
    height: 800 //Screen.height
    visible: true
    title: "Flood Watcher"

    property var selectedStation: null

    Plugin {
        id: mapPlugin
        name: "osm"
        PluginParameter {
            name: "osm.mapping.custom.host"
            value: "https://tile.openstreetmap.org/"
        }
    }

    Row {
        anchors.fill: parent

        MapArea {
            id: mapArea
            width: parent.width * 0.7
            height: parent.height

            // keep the map in sync with the app-level selection
            selectedStation: root.selectedStation

            onStationSelected: {
                root.selectedStation = station
            }
        }

        RightPanel {
            id: rightPanel
            width: parent.width * 0.3
            height: parent.height
            selectedStation: root.selectedStation
        }
    }
}
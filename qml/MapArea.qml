import QtQuick 2.15
import QtLocation 5.15
import QtPositioning 5.15

Item {
    id: root
    property var selectedStation: null
    signal stationSelected(var station)       // emitted when a marker is clicked

    Plugin {
        id: mapPlugin
        name: "osm"
        PluginParameter { name: "osm.mapping.custom.host"; value: "https://tile.openstreetmap.org/" }
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        center: QtPositioning.coordinate(53.0, -1.4)
        zoomLevel: 6.125
        activeMapType: map.supportedMapTypes[map.supportedMapTypes.length - 1]
        property geoCoordinate startCentroid

        MapItemView {
            model: stationModel
            delegate: MapQuickItem {
                coordinate: QtPositioning.coordinate(model.latitude, model.longitude)
                anchorPoint: Qt.point(sourceItem.width / 2, sourceItem.height / 2)

                sourceItem: Rectangle {
                    id: markerRect
                    width: 10; height: 10; radius: 5
                    border.color: "white"; border.width: 1
                    color: (root.selectedStation && root.selectedStation.index === index) ? "red" : "blue"

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            // emit the full data map from C++ model to dashboard
                            var station = stationModel.getStation(index);
                            root.stationSelected({index: index, data: station});
                        }
                    }
                }
            }
        }

        // map handlers
        PinchHandler {
            id: pinch; target: null
            onActiveChanged: if (active) map.startCentroid = map.toCoordinate(pinch.centroid.position, false)
            onScaleChanged: (delta) => { map.zoomLevel += Math.log2(delta); map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position) }
            onRotationChanged: (delta) => { map.bearing -= delta; map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position) }
            grabPermissions: PointerHandler.TakeOverForbidden
        }
        WheelHandler {
            id: wheel; acceptedDevices: Qt.platform.pluginName === "cocoa" || Qt.platform.pluginName === "wayland"
                             ? PointerDevice.Mouse | PointerDevice.TouchPad
                             : PointerDevice.Mouse
            rotationScale: 1/120
            property: "zoomLevel"
        }
        DragHandler { id: drag; target: null; onTranslationChanged: (delta) => map.pan(-delta.x, -delta.y) }

        Shortcut { enabled: map.zoomLevel < map.maximumZoomLevel; sequence: StandardKey.ZoomIn; onActivated: map.zoomLevel = Math.round(map.zoomLevel + 1) }
        Shortcut { enabled: map.zoomLevel > map.minimumZoomLevel; sequence: StandardKey.ZoomOut; onActivated: map.zoomLevel = Math.round(map.zoomLevel - 1) }
    }
}

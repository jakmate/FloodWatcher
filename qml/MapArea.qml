import QtQuick 2.15
import QtLocation 5.15
import QtPositioning 5.15

Item {
    id: root
    property var selectedStation: null
    signal stationSelected(var station)
    readonly property real minLat: 53.5
    readonly property real maxLat: 58.0
    readonly property real minLon: -6.0
    readonly property real maxLon: -1.6
    readonly property real minZoom: 6.125
    readonly property real maxZoom: 15.0
    
    function panToCoordinate(lat, lon, zoom) {
        map.center = QtPositioning.coordinate(lat, lon)
        if (zoom !== undefined) {
            map.zoomLevel = zoom
        }
    }

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

        // Flood warning polygons
        MapItemView {
            model: floodWarningModel
            delegate: MapPolygon {
                path: model.polygonPath
                property int sev: Number(model.severityLevel)

                color: sev === 1 ? Qt.rgba(1, 0, 0, 0.5)
                    : sev === 2 ? Qt.rgba(1, 165/255, 0, 0.5)
                    : sev === 3 ? Qt.rgba(1, 1, 0, 0.5)
                    : Qt.rgba(0, 0, 1, 0.5)

                border.color: sev === 1 ? "#C80000"
                            : sev === 2 ? "#C86400"
                            : sev === 3 ? "#C8C800"
                            : "#0000C8"

                border.width: 3
                opacity: 1
            }
        }

        // Station markers
        MapItemView {
            model: stationModel
            delegate: MapQuickItem {
                coordinate: QtPositioning.coordinate(model.latitude, model.longitude)
                anchorPoint: Qt.point(sourceItem.width / 2, sourceItem.height / 2)

                sourceItem: Rectangle {
                    id: markerRect
                    width: 8; height: 8; radius: 5
                    border.color: "white"; border.width: 1
                    color: {
                        if (root.selectedStation && root.selectedStation.index === index) {
                            return "red"
                        }
                        return "gray"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            var station = stationModel.getStation(index);
                            root.stationSelected({index: index, data: station});
                        }
                    }
                }
            }
        }

        // Map handlers
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
            onWheel: (event) => {
                var newZoom = map.zoomLevel + event.angleDelta.y * rotationScale
                map.zoomLevel = Math.max(root.minZoom, Math.min(root.maxZoom, newZoom))
                event.accepted = true
            }
        }
        DragHandler {
            id: drag
            target: null
            onTranslationChanged: (delta) => {
                var newCenter = map.toCoordinate(
                    Qt.point(map.width/2 - delta.x, map.height/2 - delta.y)
                )
                
                // Calculate dynamic bounds based on zoom level
                var zoomFactor = (map.zoomLevel - root.minZoom) / (root.maxZoom - root.minZoom)
                var latPadding = (maxLat - minLat) * zoomFactor * 3
                var lonPadding = (maxLon - minLon) * zoomFactor * 3
                
                var dynamicMinLat = minLat - latPadding
                var dynamicMaxLat = maxLat + latPadding
                var dynamicMinLon = minLon - lonPadding
                var dynamicMaxLon = maxLon + lonPadding
                
                // Clamp to dynamic bounds
                var clampedLat = Math.max(dynamicMinLat, Math.min(dynamicMaxLat, newCenter.latitude))
                var clampedLon = Math.max(dynamicMinLon, Math.min(dynamicMaxLon, newCenter.longitude))
                
                map.center = QtPositioning.coordinate(clampedLat, clampedLon)
            }
        }

        Shortcut { enabled: map.zoomLevel < map.maximumZoomLevel; sequence: StandardKey.ZoomIn; onActivated: map.zoomLevel = Math.round(map.zoomLevel + 1) }
        Shortcut { enabled: map.zoomLevel > map.minimumZoomLevel; sequence: StandardKey.ZoomOut; onActivated: map.zoomLevel = Math.round(map.zoomLevel - 1) }
    }
}
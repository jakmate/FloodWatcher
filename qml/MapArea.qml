pragma ComponentBehavior: Bound
import QtLocation
import QtPositioning
import QtQuick

Item {
    id: root

    property var stationModel: null
    property var floodWarningModel: null
    property var selectedStation: null
    readonly property real minLat: 53.5
    readonly property real maxLat: 58
    readonly property real minLon: -6
    readonly property real maxLon: -1.6
    readonly property real minZoom: 6.125
    readonly property real maxZoom: 15

    signal stationSelected(var station)
    signal polygonClicked(int warningIndex)

    function panToCoordinate(lat, lon, zoom) {
        map.center = QtPositioning.coordinate(lat, lon);
        if (zoom !== undefined)
            map.zoomLevel = zoom;
    }

    Plugin {
        id: mapPlugin

        name: "osm"

        PluginParameter {
            name: "osm.mapping.custom.host"
            value: "https://tile.openstreetmap.org/"
        }
    }

    Map {
        id: map

        property geoCoordinate startCentroid

        anchors.fill: parent
        plugin: mapPlugin
        center: QtPositioning.coordinate(53, -1.4)
        zoomLevel: 6.125
        activeMapType: map.supportedMapTypes[map.supportedMapTypes.length - 1]

        // Flood warning polygons
        MapItemView {
            id: floodWarningView
            model: root.floodWarningModel

            delegate: MapPolygon {
                id: polygonDelegate
                required property var model
                required property int index

                property int sev: Number(polygonDelegate.model.severityLevel)

                path: polygonDelegate.model.polygonPath
                color: sev === 1 ? Qt.rgba(1, 0.27, 0.27, 0.5) : sev === 2 ? Qt.rgba(1, 0.6, 0.27, 0.5) : sev === 3 ? Qt.rgba(1, 0.87, 0.27, 0.5) : Qt.rgba(0.53, 0.53, 0.53, 0.5)
                border.color: sev === 1 ? "#ff4444" : sev === 2 ? "#ff9944" : sev === 3 ? "#ffdd44" : "#888888"
                border.width: 3
                opacity: 1

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.polygonClicked(polygonDelegate.index);
                    }
                }
            }
        }

        // Station markers
        MapItemView {
            id: stationView
            model: root.stationModel

            delegate: MapQuickItem {
                id: stationDelegate
                required property var model
                required property int index

                coordinate: QtPositioning.coordinate(stationDelegate.model.latitude, stationDelegate.model.longitude)
                anchorPoint: Qt.point(sourceItem.width / 2, sourceItem.height / 2)

                sourceItem: Rectangle {
                    id: markerRect

                    width: 8
                    height: 8
                    radius: 5
                    border.color: "white"
                    border.width: 1
                    color: {
                        if (root.selectedStation && root.selectedStation.index === stationDelegate.index)
                            return "red";

                        return "black";
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            var station = root.stationModel.getStation(stationDelegate.index);
                            root.stationSelected({
                                "index": stationDelegate.index,
                                "data": station
                            });
                        }
                    }
                }
            }
        }

        // Map handlers
        PinchHandler {
            id: pinch

            target: null
            onActiveChanged: {
                if (active)
                    map.startCentroid = map.toCoordinate(pinch.centroid.position, false);
            }
            onScaleChanged: delta => {
                map.zoomLevel += Math.log2(delta);
                map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position);
            }
            onRotationChanged: delta => {
                map.bearing -= delta;
                map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position);
            }
            grabPermissions: PointerHandler.TakeOverForbidden
        }

        WheelHandler {
            id: wheel

            acceptedDevices: Qt.platform.pluginName === "cocoa" || Qt.platform.pluginName === "wayland" ? PointerDevice.Mouse | PointerDevice.TouchPad : PointerDevice.Mouse
            rotationScale: 1 / 120
            property: "zoomLevel"
            onWheel: event => {
                var newZoom = map.zoomLevel + event.angleDelta.y * rotationScale;
                map.zoomLevel = Math.max(root.minZoom, Math.min(root.maxZoom, newZoom));
                event.accepted = true;
            }
        }

        DragHandler {
            id: drag

            target: null
            onTranslationChanged: delta => {
                var newCenter = map.toCoordinate(Qt.point(map.width / 2 - delta.x, map.height / 2 - delta.y));
                // Calculate dynamic bounds based on zoom level
                var zoomFactor = (map.zoomLevel - root.minZoom) / (root.maxZoom - root.minZoom);
                var latPadding = (root.maxLat - root.minLat) * zoomFactor * 3;
                var lonPadding = (root.maxLon - root.minLon) * zoomFactor * 3;
                var dynamicMinLat = root.minLat - latPadding;
                var dynamicMaxLat = root.maxLat + latPadding;
                var dynamicMinLon = root.minLon - lonPadding;
                var dynamicMaxLon = root.maxLon + lonPadding;
                // Clamp to dynamic bounds
                var clampedLat = Math.max(dynamicMinLat, Math.min(dynamicMaxLat, newCenter.latitude));
                var clampedLon = Math.max(dynamicMinLon, Math.min(dynamicMaxLon, newCenter.longitude));
                map.center = QtPositioning.coordinate(clampedLat, clampedLon);
            }
        }

        Shortcut {
            enabled: map.zoomLevel < map.maximumZoomLevel
            sequence: StandardKey.ZoomIn
            onActivated: map.zoomLevel = Math.round(map.zoomLevel + 1)
        }

        Shortcut {
            enabled: map.zoomLevel > map.minimumZoomLevel
            sequence: StandardKey.ZoomOut
            onActivated: map.zoomLevel = Math.round(map.zoomLevel - 1)
        }
    }
}

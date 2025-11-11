import QtQuick 2.15
import QtLocation 5.15
import QtPositioning 5.15
import QtQuick.Window 2.15

Window {
    id: root
    width: 800
    height: 800
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

    // Helper function to calculate centroid of a polygon
    function calculateCentroid(polygonPath) {
        if (!polygonPath || polygonPath.length === 0) return null
        
        var latSum = 0
        var lonSum = 0
        var count = polygonPath.length
        
        for (var i = 0; i < count; i++) {
            var coord = polygonPath[i]
            latSum += coord.latitude
            lonSum += coord.longitude
        }
        
        return {
            lat: latSum / count,
            lon: lonSum / count
        }
    }
    
    // Calculate appropriate zoom based on polygon size
    function calculateZoomForPolygon(polygonPath) {
        if (!polygonPath || polygonPath.length === 0) return 10
        
        var minLat = polygonPath[0].latitude
        var maxLat = polygonPath[0].latitude
        var minLon = polygonPath[0].longitude
        var maxLon = polygonPath[0].longitude
        
        for (var i = 1; i < polygonPath.length; i++) {
            var coord = polygonPath[i]
            minLat = Math.min(minLat, coord.latitude)
            maxLat = Math.max(maxLat, coord.latitude)
            minLon = Math.min(minLon, coord.longitude)
            maxLon = Math.max(maxLon, coord.longitude)
        }
        
        var latSpan = maxLat - minLat
        var lonSpan = maxLon - minLon
        var maxSpan = Math.max(latSpan, lonSpan)
        
        // Larger span = lower zoom, smaller span = higher zoom
        if (maxSpan > 1.0) return 8
        if (maxSpan > 0.5) return 9
        if (maxSpan > 0.25) return 10
        if (maxSpan > 0.1) return 11
        if (maxSpan > 0.05) return 12
        if (maxSpan > 0.02) return 13
        return 14
    }

    Row {
        anchors.fill: parent

        MapArea {
            id: mapArea
            width: parent.width * 0.7
            height: parent.height

            selectedStation: root.selectedStation

            onStationSelected: (station) => {
                root.selectedStation = station
            }

            onPolygonClicked: (warningIndex) => {
                rightPanel.scrollToWarning(warningIndex)
            }
        }

        RightPanel {
            id: rightPanel
            width: parent.width * 0.3
            height: parent.height
            selectedStation: root.selectedStation
            
            onFloodWarningClicked: (polygonPath) => {
                var centroid = calculateCentroid(polygonPath)
                if (centroid) {
                    var zoom = calculateZoomForPolygon(polygonPath)
                    mapArea.panToCoordinate(centroid.lat, centroid.lon, zoom)
                }
            }
            
            onStationClosed: {
                root.selectedStation = null
            }
        }
    }
}
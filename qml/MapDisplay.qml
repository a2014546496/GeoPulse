import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning

Item {
    id: mapDisplay

    // ── Follow mode ─────────────────────────────────────
    property bool followMode: true

    // ── Track state ─────────────────────────────────────
    property var trackPoints: []
    property double lastLat: 0
    property double lastLon: 0

    // ── Native QtLocation Map ──────────────────────────
    Map {
        id: qtMap
        anchors.fill: parent

        plugin: Plugin {
            name: "osm"
            PluginParameter { name: "osm.mapping.providersrepository.disabled"; value: "true" }
            PluginParameter {
                name: "osm.mapping.providersrepository.address"
                value: "https://tile.openstreetmap.org/"
            }
        }
        center: QtPositioning.coordinate(34, 108)
        zoomLevel: 5
        activeMapType: supportedMapTypes[0]

        // ── Gesture handlers ────────────────────────────
        PinchHandler {
            id: pinch
            target: null
            property real rawBearing: 0
            property var startCentroid
            onActiveChanged: {
                if (active) {
                    pinch.startCentroid = qtMap.toCoordinate(pinch.centroid.position, false)
                }
            }
            onScaleChanged: (delta) => {
                qtMap.zoomLevel += Math.log2(delta)
                qtMap.alignCoordinateToPoint(pinch.startCentroid, pinch.centroid.position)
            }
            onRotationChanged: (delta) => {
                pinch.rawBearing -= delta
                qtMap.bearing = (Math.abs(pinch.rawBearing) < 5) ? 0 : pinch.rawBearing
                qtMap.alignCoordinateToPoint(pinch.startCentroid, pinch.centroid.position)
            }
            grabPermissions: PointerHandler.TakeOverForbidden
        }

        DragHandler {
            id: drag
            target: null
            onTranslationChanged: (delta) => qtMap.pan(-delta.x, -delta.y)
        }

        WheelHandler {
            id: wheel
            acceptedDevices: PointerDevice.Mouse
            onWheel: (event) => {
                qtMap.zoomLevel += event.angleDelta.y / 120
            }
        }

        // ── Position marker ────────────────────────────
        MapQuickItem {
            id: positionMarker
            coordinate: QtPositioning.coordinate(GpsManager.latitude, GpsManager.longitude)
            visible: GpsManager.positionValid
            anchorPoint.x: 8
            anchorPoint.y: 8
            z: 100

            sourceItem: Rectangle {
                width: 16; height: 16; radius: 8
                color: "#00e5ff"
                border.color: "white"
                border.width: 2
            }
        }

        // ── Accuracy circle ────────────────────────────
        MapCircle {
            id: accuracyCircle
            center: QtPositioning.coordinate(GpsManager.latitude, GpsManager.longitude)
            visible: GpsManager.positionValid
            radius: Math.max(GpsManager.hdop * 5, 10)
            color: Qt.rgba(0, 0.9, 1, 0.15)
            border.color: Qt.rgba(0, 0.9, 1, 0.4)
            border.width: 1
        }

        // ── Track polyline ─────────────────────────────
        MapPolyline {
            id: trackLine
            line.width: 3
            line.color: "#ff9800"
            opacity: 0.9
        }
    }

    // ── Follow mode Binding ─────────────────────────────
    Binding {
        target: qtMap
        property: "center"
        value: QtPositioning.coordinate(GpsManager.latitude, GpsManager.longitude)
        when: mapDisplay.followMode && GpsManager.positionValid
        delayed: true
    }

    // ── Fence model for Instantiator ────────────────────
    ListModel {
        id: fenceModel
    }

    // ── Geofence polygons ──────────────────────────────
    Instantiator {
        model: fenceModel
        delegate: MapPolygon {
            id: fencePoly
            required property var fencePath
            path: fencePath
            color: Qt.rgba(1, 0.27, 0.27, 0.2)
            border.color: "#ff4444"
            border.width: 2
        }
        onObjectAdded: (index, object) => {
            qtMap.addMapItem(object)
        }
        onObjectRemoved: (index, object) => {
            qtMap.removeMapItem(object)
        }
    }

    // ── Map controls overlay ───────────────────────────
    Column {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 10
        spacing: 6
        z: 10

        // Zoom in
        Rectangle {
            width: 36; height: 36; radius: 18
            color: "#16213e"; opacity: 0.9
            border.color: "#3a506b"; border.width: 1
            Text {
                anchors.centerIn: parent
                text: "+"; color: "#e0e0e0"; font.pixelSize: 22; font.bold: true
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: qtMap.zoomLevel += 1
            }
        }

        // Zoom out
        Rectangle {
            width: 36; height: 36; radius: 18
            color: "#16213e"; opacity: 0.9
            border.color: "#3a506b"; border.width: 1
            Text {
                anchors.centerIn: parent
                text: "−"; color: "#e0e0e0"; font.pixelSize: 22; font.bold: true
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: qtMap.zoomLevel -= 1
            }
        }

        // Follow mode toggle
        Rectangle {
            id: followBtn
            width: 36; height: 36; radius: 18
            color: mapDisplay.followMode ? "#0d47a1" : "#16213e"
            opacity: 0.9
            border.color: "#3a506b"; border.width: 1
            Text {
                anchors.centerIn: parent
                text: "⊙"; color: "#e0e0e0"; font.pixelSize: 18
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: mapDisplay.followMode = !mapDisplay.followMode
            }
        }

        // Center on position
        Rectangle {
            width: 36; height: 36; radius: 18
            color: "#16213e"; opacity: 0.9
            border.color: "#3a506b"; border.width: 1
            Text {
                anchors.centerIn: parent
                text: "◎"; color: "#e0e0e0"; font.pixelSize: 16
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (GpsManager.positionValid) {
                        qtMap.center = QtPositioning.coordinate(
                            GpsManager.latitude, GpsManager.longitude)
                        qtMap.zoomLevel = 16
                    }
                }
            }
        }
    }

    // ── Position Update Timer ──────────────────────────
    // Used for track recording incremental updates
    Timer {
        id: positionUpdateTimer
        interval: 500
        running: true
        repeat: true

        onTriggered: {
            if (!GpsManager.positionValid) return

            var lat = GpsManager.latitude
            var lon = GpsManager.longitude

            // Add track point if recording and position changed
            if (TrackRecorder.recording) {
                if (Math.abs(lat - mapDisplay.lastLat) > 0.00001 ||
                    Math.abs(lon - mapDisplay.lastLon) > 0.00001) {
                    // Append to track path
                    var newPath = trackLine.path
                    newPath.push(QtPositioning.coordinate(lat, lon))
                    trackLine.path = newPath
                    mapDisplay.lastLat = lat
                    mapDisplay.lastLon = lon
                }
            }
        }
    }

    // ── TrackRecorder connections ──────────────────────
    Connections {
        target: TrackRecorder
        function onRecordingChanged(recording) {
            if (!recording) {
                // Recording stopped: load simplified track
                var pts = TrackRecorder.simplifiedTrack(2.0)
                var coords = []
                for (var i = 0; i < pts.length; i++) {
                    coords.push(QtPositioning.coordinate(pts[i].lat, pts[i].lon))
                }
                trackLine.path = coords
            } else {
                // Recording started: clear track
                trackLine.path = []
            }
        }
    }

    // ── GeofenceManager connections ────────────────────
    Connections {
        target: GeofenceManager
        function onFencesChanged() {
            fenceModel.clear()
            var fences = GeofenceManager.fenceList()
            for (var i = 0; i < fences.length; i++) {
                var f = fences[i]
                var coords = []
                for (var j = 0; j < f.points.length; j++) {
                    coords.push(QtPositioning.coordinate(
                        f.points[j].lat, f.points[j].lon))
                }
                fenceModel.append({ fencePath: coords })
            }
        }
    }
}

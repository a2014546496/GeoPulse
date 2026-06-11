import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 1280
    height: 800
    title: "GeoPulse — GPS Monitor"

    color: "#1a1a2e"

    // ── Top toolbar ──────────────────────────────────
    header: ToolBar {
        id: toolbar
        background: Rectangle { color: "#16213e" }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12

            Text {
                text: "🌍 GeoPulse"
                font.pixelSize: 20
                font.bold: true
                color: "#e0e0e0"
                Layout.alignment: Qt.AlignVCenter
            }

            Item { Layout.fillWidth: true }

            // Connection indicator
            Rectangle {
                id: connectionDot
                width: 10; height: 10; radius: 5
                color: GpsManager.connected ? "#4caf50" : "#f44336"
                Layout.alignment: Qt.AlignVCenter
            }
            Text {
                text: GpsManager.connected ? GpsManager.driverType + " Connected" : "Disconnected"
                color: GpsManager.connected ? "#81c784" : "#ef9a9a"
                font.pixelSize: 12
                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: 16
            }

            // Fix quality indicator
            Rectangle {
                radius: 4
                color: {
                    switch(GpsManager.fixQuality) {
                        case 0: return "#555555";
                        case 1: return "#ff9800";
                        case 2: return "#ffc107";
                        case 4: return "#4caf50";
                        case 5: return "#2196f3";
                        default: return "#555555";
                    }
                }
                width: fixLabel.implicitWidth + 16; height: 24
                Layout.alignment: Qt.AlignVCenter
                Text {
                    id: fixLabel
                    anchors.centerIn: parent
                    text: {
                        switch(GpsManager.fixQuality) {
                            case 0: return "No Fix";
                            case 1: return "GPS Fix";
                            case 2: return "DGPS";
                            case 4: return "RTK Fixed";
                            case 5: return "RTK Float";
                            default: return "Fix: " + GpsManager.fixQuality;
                        }
                    }
                    color: "white"
                    font.pixelSize: 11
                    font.bold: true
                }
            }

            Text {
                text: "🛰 " + GpsManager.satelliteCount
                color: "#b0bec5"
                font.pixelSize: 13
                Layout.leftMargin: 12
            }
        }
    }

    // ── Geofence Alert Toast ──────────────────────────
    Rectangle {
        id: geofenceToast
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 56
        width: geofenceToastText.implicitWidth + 40
        height: 36
        radius: 8
        color: "#e65100"
        opacity: 0.0
        z: 1000

        Behavior on opacity { NumberAnimation { duration: 300 } }

        Text {
            id: geofenceToastText
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: 14
            font.bold: true
        }
    }

    Timer {
        id: geofenceDismissTimer
        interval: 3000
        onTriggered: geofenceToast.opacity = 0.0
    }

    // ── Main layout ──────────────────────────────────
    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Map view (left, 65%)
        MapDisplay {
            id: mapDisplay
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: 400
        }

        // Side panel (right, 35%)
        Rectangle {
            Layout.preferredWidth: 380
            Layout.fillHeight: true
            color: "#1e2a3a"

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // Tab bar
                TabBar {
                    id: sideTabBar
                    Layout.fillWidth: true
                    background: Rectangle { color: "#16213e" }
                    spacing: 0

                    TabButton {
                        text: "Dashboard"
                        implicitHeight: 36
                        font.pixelSize: 12
                        palette { buttonText: checked ? "#ffffff" : "#78909c" }
                        background: Rectangle {
                            color: parent.checked ? "#0d47a1" : "transparent"
                        }
                    }
                    TabButton {
                        text: "Satellites"
                        implicitHeight: 36
                        font.pixelSize: 12
                        palette { buttonText: checked ? "#ffffff" : "#78909c" }
                        background: Rectangle {
                            color: parent.checked ? "#0d47a1" : "transparent"
                        }
                    }
                    TabButton {
                        text: "Track"
                        implicitHeight: 36
                        font.pixelSize: 12
                        palette { buttonText: checked ? "#ffffff" : "#78909c" }
                        background: Rectangle {
                            color: parent.checked ? "#0d47a1" : "transparent"
                        }
                    }
                    TabButton {
                        text: "Settings"
                        implicitHeight: 36
                        font.pixelSize: 12
                        palette { buttonText: checked ? "#ffffff" : "#78909c" }
                        background: Rectangle {
                            color: parent.checked ? "#0d47a1" : "transparent"
                        }
                    }
                }

                // Stack
                SwipeView {
                    id: sideStack
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: sideTabBar.currentIndex
                    onCurrentIndexChanged: sideTabBar.currentIndex = sideStack.currentIndex
                    interactive: true
                    clip: true

                    Dashboard { }
                    SatellitePanel { }
                    TrackPanel { }
                    SettingsPanel { }
                }
            }
        }
    }

    // ── Status bar ───────────────────────────────────
    footer: Rectangle {
        height: 24
        color: "#16213e"

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 16

            Text { text: GpsManager.positionValid ?
                         GpsManager.latitude.toFixed(6) + "°, " + GpsManager.longitude.toFixed(6) + "°" :
                         "Waiting for position..."
                  color: "#90a4ae"; font.pixelSize: 11 }
        }
    }

    // ── GeofenceManager connections ──────────────────
    Connections {
        target: GeofenceManager
        function onEnteredGeofence(name) {
            geofenceToastText.text = "📍 Entered: " + name
            geofenceToast.opacity = 1.0
            geofenceDismissTimer.restart()
        }
        function onExitedGeofence(name) {
            geofenceToastText.text = "📍 Exited: " + name
            geofenceToast.opacity = 1.0
            geofenceDismissTimer.restart()
        }
    }

    // ── Initialize ──────────────────────────────────
    Component.onCompleted: {
        // Try to auto-connect if config has saved settings
        // The GeoPulseApp.initialize() handles this
    }

    onClosing: {
        App.shutdown()
    }
}

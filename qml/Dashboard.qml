import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: dashboard

    ScrollView {
        anchors.fill: parent
        anchors.margins: 10
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 12

            // ── Position ──────────────────────────────────
            GroupBox {
                title: "Position"
                Layout.fillWidth: true
                label: Text {
                    text: "📍 Position"
                    color: "#90caf9"
                    font.pixelSize: 13
                    font.bold: true
                }
                background: Rectangle {
                    color: "#162447"
                    radius: 6
                    border.color: "#1f4068"
                }

                GridLayout {
                    columns: 2
                    columnSpacing: 12
                    rowSpacing: 6
                    anchors.fill: parent

                    Text { text: "Latitude:"; color: "#78909c"; font.pixelSize: 12 }
                    Text {
                        text: GpsManager.positionValid ?
                                  GpsManager.latitude.toFixed(6) + "°" : "--"
                        color: "#e0e0e0"
                        font.pixelSize: 14
                        font.family: "monospace"
                    }

                    Text { text: "Longitude:"; color: "#78909c"; font.pixelSize: 12 }
                    Text {
                        text: GpsManager.positionValid ?
                                  GpsManager.longitude.toFixed(6) + "°" : "--"
                        color: "#e0e0e0"
                        font.pixelSize: 14
                        font.family: "monospace"
                    }

                    Text { text: "Altitude:"; color: "#78909c"; font.pixelSize: 12 }
                    Text {
                        text: GpsManager.positionValid ?
                                  GpsManager.altitude.toFixed(1) + " m" : "--"
                        color: "#e0e0e0"
                        font.pixelSize: 14
                        font.family: "monospace"
                    }
                }
            }

            // ── Movement ──────────────────────────────────
            GroupBox {
                title: "Movement"
                Layout.fillWidth: true
                label: Text {
                    text: "🚀 Movement"
                    color: "#90caf9"
                    font.pixelSize: 13
                    font.bold: true
                }
                background: Rectangle {
                    color: "#162447"
                    radius: 6
                    border.color: "#1f4068"
                }

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10

                    // Speed gauge
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        // Speed value
                        Column {
                            Text {
                                text: GpsManager.positionValid ?
                                          GpsManager.speed.toFixed(1) : "0.0"
                                color: "#00e5ff"
                                font.pixelSize: 42
                                font.bold: true
                                font.family: "monospace"
                            }
                            Text {
                                text: "km/h"
                                color: "#78909c"
                                font.pixelSize: 12
                            }
                        }

                        // Speed bar
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 12
                            radius: 6
                            color: "#0d2137"

                            Rectangle {
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                width: parent.width * Math.min(
                                           GpsManager.speed / 120.0, 1.0)
                                radius: 6
                                color: {
                                    var s = GpsManager.speed
                                    if (s < 30) return "#4caf50"
                                    if (s < 80) return "#ff9800"
                                    return "#f44336"
                                }
                            }
                        }
                    }

                    // Course
                    RowLayout {
                        Text { text: "Course:"; color: "#78909c"; font.pixelSize: 12 }
                        Text {
                            text: GpsManager.positionValid ?
                                      GpsManager.course.toFixed(1) + "°" : "--"
                            color: "#e0e0e0"
                            font.pixelSize: 14
                            font.family: "monospace"
                        }
                        Layout.fillWidth: true

                        // Simple direction indicator
                        Text {
                            text: {
                                var c = GpsManager.course
                                if (!GpsManager.positionValid) return "—"
                                var dirs = ["N", "NE", "E", "SE", "S", "SW", "W", "NW"]
                                var idx = Math.round(c / 45) % 8
                                return dirs[idx]
                            }
                            color: "#ffab40"
                            font.pixelSize: 14
                            font.bold: true
                        }
                    }
                }
            }

            // ── Signal Quality ────────────────────────────
            GroupBox {
                title: "Signal Quality"
                Layout.fillWidth: true
                label: Text {
                    text: "📶 Signal"
                    color: "#90caf9"
                    font.pixelSize: 13
                    font.bold: true
                }
                background: Rectangle {
                    color: "#162447"
                    radius: 6
                    border.color: "#1f4068"
                }

                GridLayout {
                    columns: 2
                    columnSpacing: 12
                    rowSpacing: 6
                    anchors.fill: parent

                    Text { text: "Fix Quality:"; color: "#78909c"; font.pixelSize: 12 }
                    Text {
                        text: {
                            switch(GpsManager.fixQuality) {
                                case 0: return "Invalid"
                                case 1: return "GPS Fix"
                                case 2: return "DGPS"
                                case 4: return "RTK Fixed"
                                case 5: return "RTK Float"
                                default: return "Unknown (" + GpsManager.fixQuality + ")"
                            }
                        }
                        color: GpsManager.fixQuality >= 1 ? "#81c784" : "#ef9a9a"
                        font.pixelSize: 13
                        font.bold: true
                    }

                    Text { text: "Satellites:"; color: "#78909c"; font.pixelSize: 12 }
                    Text {
                        text: GpsManager.satelliteCount + " 🛰"
                        color: "#e0e0e0"
                        font.pixelSize: 14
                        font.family: "monospace"
                    }

                    Text { text: "HDOP:"; color: "#78909c"; font.pixelSize: 12 }
                    Text {
                        text: GpsManager.positionValid ?
                                  GpsManager.hdop.toFixed(2) : "--"
                        color: {
                            var h = GpsManager.hdop
                            if (h < 1.0) return "#81c784"
                            if (h < 2.0) return "#ffc107"
                            return "#ef9a9a"
                        }
                        font.pixelSize: 14
                        font.family: "monospace"
                    }

                    Text { text: "Sat in View:"; color: "#78909c"; font.pixelSize: 12 }
                    Text {
                        text: GpsManager.satelliteCount + " sats"
                        color: "#e0e0e0"
                        font.pixelSize: 14
                        font.family: "monospace"
                    }
                }
            }
        }
    }
}

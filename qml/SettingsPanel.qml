import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: settingsPanel

    ScrollView {
        anchors.fill: parent
        anchors.margins: 10
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 12

            // ── Connection ───────────────────────────────
            GroupBox {
                title: "GPS Connection"
                Layout.fillWidth: true
                label: Text {
                    text: "🔌 Connection"
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

                    // Driver type selector
                    RowLayout {
                        Text { text: "Type:"; color: "#78909c"; font.pixelSize: 12; Layout.preferredWidth: 70 }
                        ComboBox {
                            id: driverTypeCombo
                            model: ["Serial", "TCP", "UDP"]
                            currentIndex: 0
                            Layout.fillWidth: true
                            palette {
                                base: "#0d2137"
                                text: "#e0e0e0"
                                buttonText: "#e0e0e0"
                            }
                        }
                    }

                    // Address
                    RowLayout {
                        Text { text: "Address:"; color: "#78909c"; font.pixelSize: 12; Layout.preferredWidth: 70 }
                        TextField {
                            id: addressField
                            Layout.fillWidth: true
                            text: driverTypeCombo.currentText === "Serial" ?
                                      "/dev/ttyUSB0" : "127.0.0.1:5000"
                            palette {
                                base: "#0d2137"
                                text: "#e0e0e0"
                                placeholderText: "#546e7a"
                            }
                            font.family: "monospace"
                            font.pixelSize: 12
                        }
                    }

                    // Baud rate (serial only)
                    RowLayout {
                        visible: driverTypeCombo.currentText === "Serial"
                        Text { text: "Baud:"; color: "#78909c"; font.pixelSize: 12; Layout.preferredWidth: 70 }
                        ComboBox {
                            id: baudCombo
                            model: ["4800", "9600", "19200", "38400", "57600", "115200"]
                            currentIndex: 1
                            Layout.fillWidth: true
                            palette {
                                base: "#0d2137"
                                text: "#e0e0e0"
                                buttonText: "#e0e0e0"
                            }
                        }
                    }

                    // Connect / Disconnect button
                    Button {
                        id: connectBtn
                        Layout.fillWidth: true
                        Layout.preferredHeight: 38
                        text: GpsManager.connected ? "Disconnect" : "Connect"

                        palette {
                            buttonText: "white"
                            button: GpsManager.connected ? "#c62828" : "#2e7d32"
                        }

                        onClicked: {
                            if (GpsManager.connected) {
                                GpsManager.disconnectFromSource()
                            } else {
                                GpsManager.connectToSource(
                                    addressField.text,
                                    driverTypeCombo.currentText.toLowerCase(),
                                    parseInt(baudCombo.currentText)
                                )
                            }
                        }
                    }

                    // Status message
                    Text {
                        text: GpsManager.connected ?
                                  "Connected via " + GpsManager.driverType :
                                  "Not connected"
                        color: GpsManager.connected ? "#81c784" : "#78909c"
                        font.pixelSize: 12
                    }
                }
            }

            // ── Map Settings ─────────────────────────────
            GroupBox {
                title: "Map"
                Layout.fillWidth: true
                label: Text {
                    text: "🗺 Map Settings"
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
                    spacing: 8

                    RowLayout {
                        Text { text: "Map Layer:"; color: "#78909c"; font.pixelSize: 12 }
                        ComboBox {
                            model: ["OpenStreetMap", "Satellite (TODO)"]
                            currentIndex: 0
                            Layout.fillWidth: true
                            palette {
                                base: "#0d2137"
                                text: "#e0e0e0"
                                buttonText: "#e0e0e0"
                            }
                        }
                    }

                    CheckBox {
                        text: "Auto-follow position"
                        checked: true
                        palette { buttonText: "#e0e0e0" }
                    }
                }
            }

            // ── Logging ──────────────────────────────────
            GroupBox {
                title: "Logging"
                Layout.fillWidth: true
                label: Text {
                    text: "📝 Logging"
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
                    spacing: 8

                    RowLayout {
                        Text { text: "Level:"; color: "#78909c"; font.pixelSize: 12 }
                        ComboBox {
                            model: ["Debug", "Info", "Warning", "Error"]
                            currentIndex: 1
                            Layout.fillWidth: true
                            palette {
                                base: "#0d2137"
                                text: "#e0e0e0"
                                buttonText: "#e0e0e0"
                            }
                        }
                    }

                    CheckBox {
                        text: "Show raw NMEA in console"
                        checked: false
                        palette { buttonText: "#e0e0e0" }
                    }
                }
            }

            // ── About ────────────────────────────────────
            GroupBox {
                title: "About"
                Layout.fillWidth: true
                label: Text {
                    text: "ℹ About"
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
                    spacing: 4

                    Text {
                        text: "GeoPulse v" + App.appVersion
                        color: "#e0e0e0"
                        font.pixelSize: 14
                        font.bold: true
                    }
                    Text {
                        text: "Real-time GPS Monitoring & Tracking\nBuilt with Qt 6 + QML + Leaflet"
                        color: "#78909c"
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}

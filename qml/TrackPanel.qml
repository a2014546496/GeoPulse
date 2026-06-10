import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
    id: trackPanel

    ScrollView {
        anchors.fill: parent
        anchors.margins: 10
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 12

            // ── Recording Controls ────────────────────────
            GroupBox {
                title: "Recording"
                Layout.fillWidth: true
                label: Text {
                    text: "⏺ Track Recording"
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

                    // Start/Stop button
                    RowLayout {
                        spacing: 10

                        Button {
                            text: TrackRecorder.recording ? "⏹ Stop" : "⏺ Record"
                            font.pixelSize: 13
                            palette {
                                buttonText: "white"
                                button: TrackRecorder.recording ? "#c62828" : "#2e7d32"
                            }
                            onClicked: {
                                if (TrackRecorder.recording)
                                    TrackRecorder.stopRecording()
                                else
                                    TrackRecorder.startRecording()
                            }
                        }

                        Button {
                            text: "🗑 Clear"
                            font.pixelSize: 13
                            enabled: TrackRecorder.pointCount > 0 && !TrackRecorder.recording
                            palette {
                                buttonText: "white"
                                button: "#546e7a"
                            }
                            onClicked: TrackRecorder.clearTrack()
                        }
                    }

                    // Recording indicator
                    Rectangle {
                        visible: TrackRecorder.recording
                        width: 12; height: 12; radius: 6
                        color: "#f44336"
                        SequentialAnimation on opacity {
                            running: TrackRecorder.recording
                            loops: Animation.Infinite
                            NumberAnimation { from: 1.0; to: 0.2; duration: 500 }
                            NumberAnimation { from: 0.2; to: 1.0; duration: 500 }
                        }
                    }
                }
            }

            // ── Track Statistics ──────────────────────────
            GroupBox {
                title: "Statistics"
                Layout.fillWidth: true
                label: Text {
                    text: "📈 Statistics"
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

                    Text { text: "Points:"; color: "#78909c"; font.pixelSize: 12 }
                    Text {
                        text: TrackRecorder.pointCount
                        color: "#e0e0e0"
                        font.pixelSize: 14
                        font.family: "monospace"
                    }

                    Text { text: "Distance:"; color: "#78909c"; font.pixelSize: 12 }
                    Text {
                        text: TrackRecorder.totalDistance >= 1000 ?
                                  (TrackRecorder.totalDistance / 1000).toFixed(2) + " km" :
                                  TrackRecorder.totalDistance.toFixed(1) + " m"
                        color: "#e0e0e0"
                        font.pixelSize: 14
                        font.family: "monospace"
                    }

                    Text { text: "Started:"; color: "#78909c"; font.pixelSize: 12 }
                    Text {
                        text: TrackRecorder.recording ? TrackRecorder.startTime : "--"
                        color: "#b0bec5"
                        font.pixelSize: 12
                    }

                    Text { text: "Elapsed:"; color: "#78909c"; font.pixelSize: 12 }
                    Text {
                        text: TrackRecorder.recording ? TrackRecorder.elapsedTime : "--:--:--"
                        color: "#ffab40"
                        font.pixelSize: 14
                        font.family: "monospace"
                    }

                    Text { text: "Avg Speed:"; color: "#78909c"; font.pixelSize: 12 }
                    Text {
                        text: {
                            if (TrackRecorder.pointCount < 2) return "--"
                            var secs = TrackRecorder.startTime ? new Date() / 1000 - Date.parse(TrackRecorder.startTime) / 1000 : 0
                            if (secs <= 0) return "--"
                            var avg = (TrackRecorder.totalDistance / secs) * 3.6
                            return avg.toFixed(1) + " km/h"
                        }
                        color: "#e0e0e0"
                        font.pixelSize: 14
                        font.family: "monospace"
                    }
                }
            }

            // ── Export ────────────────────────────────────
            GroupBox {
                title: "Export"
                Layout.fillWidth: true
                label: Text {
                    text: "💾 Export Track"
                    color: "#90caf9"
                    font.pixelSize: 13
                    font.bold: true
                }
                background: Rectangle {
                    color: "#162447"
                    radius: 6
                    border.color: "#1f4068"
                }

                RowLayout {
                    spacing: 10
                    anchors.fill: parent

                    Button {
                        text: "📄 Export GPX"
                        font.pixelSize: 12
                        enabled: TrackRecorder.pointCount > 0 && !TrackRecorder.recording
                        palette { buttonText: "white"; button: "#1565c0" }
                        onClicked: {
                            gpxSaveDialog.open()
                        }
                    }

                    Button {
                        text: "📊 Export CSV"
                        font.pixelSize: 12
                        enabled: TrackRecorder.pointCount > 0 && !TrackRecorder.recording
                        palette { buttonText: "white"; button: "#00695c" }
                        onClicked: {
                            csvSaveDialog.open()
                        }
                    }
                }
            }
        }
    }

    // ── File Dialogs ──────────────────────────────────
    FileDialog {
        id: gpxSaveDialog
        fileMode: FileDialog.SaveFile
        nameFilters: ["GPX files (*.gpx)", "All files (*)"]
        onAccepted: {
            TrackRecorder.exportToGpx(selectedFile)
        }
    }

    FileDialog {
        id: csvSaveDialog
        fileMode: FileDialog.SaveFile
        nameFilters: ["CSV files (*.csv)", "All files (*)"]
        onAccepted: {
            TrackRecorder.exportToCsv(selectedFile)
        }
    }
}

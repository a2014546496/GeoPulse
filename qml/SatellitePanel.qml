import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: satellitePanel

    ScrollView {
        anchors.fill: parent
        anchors.margins: 10
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 12

            // ── Sky Plot ─────────────────────────────────
            GroupBox {
                title: "Sky Plot"
                Layout.fillWidth: true
                Layout.preferredHeight: 220
                label: Text {
                    text: "🛰 Satellite Sky Plot"
                    color: "#90caf9"
                    font.pixelSize: 13
                    font.bold: true
                }
                background: Rectangle {
                    color: "#162447"
                    radius: 6
                    border.color: "#1f4068"
                }

                Canvas {
                    id: skyPlot
                    anchors.fill: parent
                    anchors.margins: 10
                    property int satCount: GpsManager.satelliteCount

                    onSatCountChanged: requestPaint()

                    onPaint: {
                        var ctx = getContext("2d")
                        var cx = width / 2
                        var cy = height / 2
                        var r = Math.min(cx, cy) - 15

                        ctx.clearRect(0, 0, width, height)

                        // Concentric circles (elevation rings)
                        for (var i = 1; i <= 3; i++) {
                            var ringR = r * i / 3
                            ctx.beginPath()
                            ctx.arc(cx, cy, ringR, 0, 2 * Math.PI)
                            ctx.strokeStyle = "#2a5078"
                            ctx.lineWidth = 1
                            ctx.stroke()

                            // Elevation label
                            var elev = 90 - (i / 3) * 90
                            ctx.fillStyle = "#617d9a"
                            ctx.font = "10px sans-serif"
                            ctx.fillText(elev + "°", cx + 3, cy - ringR + 12)
                        }

                        // Cross hairs
                        ctx.beginPath()
                        ctx.moveTo(cx - r, cy); ctx.lineTo(cx + r, cy)
                        ctx.moveTo(cx, cy - r); ctx.lineTo(cx, cy + r)
                        ctx.strokeStyle = "#2a5078"
                        ctx.lineWidth = 1
                        ctx.stroke()

                        // Direction labels
                        ctx.fillStyle = "#78909c"
                        ctx.font = "12px sans-serif"
                        ctx.fillText("N", cx - 5, cy - r - 3)
                        ctx.fillText("E", cx + r + 5, cy + 4)
                        ctx.fillText("S", cx - 5, cy + r + 14)
                        ctx.fillText("W", cx - r - 18, cy + 4)

                        // TODO: Plot real satellites from GpsData::satellitesInView
                        // This requires exposing satellite list via C++ model
                        // Placeholder satellites plotted as dots
                        for (var s = 0; s < satCount && s < 20; s++) {
                            var elev = Math.random() * 80 + 5   // placeholder
                            var azim = Math.random() * 360
                            var azRad = (90 - azim) * Math.PI / 180
                            var elevR = r * (1.0 - elev / 90.0)

                            var sx = cx + elevR * Math.cos(azRad)
                            var sy = cy - elevR * Math.sin(azRad)

                            ctx.beginPath()
                            ctx.arc(sx, sy, 4, 0, 2 * Math.PI)
                            ctx.fillStyle = "#42a5f5"
                            ctx.fill()

                            // PRN label
                            ctx.fillStyle = "#90caf9"
                            ctx.font = "8px monospace"
                            ctx.fillText(s + 1, sx + 5, sy + 3)
                        }
                    }
                }
            }

            // ── SNR Bar Chart ────────────────────────────
            GroupBox {
                title: "Signal-to-Noise Ratio"
                Layout.fillWidth: true
                Layout.preferredHeight: 140
                label: Text {
                    text: "📊 SNR (dB)"
                    color: "#90caf9"
                    font.pixelSize: 13
                    font.bold: true
                }
                background: Rectangle {
                    color: "#162447"
                    radius: 6
                    border.color: "#1f4068"
                }

                Canvas {
                    id: snrCanvas
                    anchors.fill: parent
                    anchors.margins: 10
                    property int satCount: GpsManager.satelliteCount

                    onSatCountChanged: requestPaint()

                    onPaint: {
                        var ctx = getContext("2d")
                        var w = width
                        var h = height
                        ctx.clearRect(0, 0, w, h)

                        var bars = Math.max(satCount, 4)
                        var barW = (w - (bars - 1) * 4) / bars
                        var maxSnr = 50

                        // Grid lines
                        for (var i = 0; i <= 5; i++) {
                            var y = h - (h * i / 5)
                            ctx.beginPath()
                            ctx.moveTo(0, y); ctx.lineTo(w, y)
                            ctx.strokeStyle = "#1a3650"
                            ctx.lineWidth = 1
                            ctx.stroke()

                            ctx.fillStyle = "#617d9a"
                            ctx.font = "9px sans-serif"
                            ctx.fillText(Math.round(maxSnr * i / 5) + "dB", 2, y - 3)
                        }

                        for (var b = 0; b < bars; b++) {
                            var snr = Math.random() * 45 + 5  // placeholder
                            var barH = h * (snr / maxSnr)
                            var bx = b * (barW + 4)
                            var by = h - barH

                            ctx.fillStyle = snr > 35 ? "#4caf50" :
                                            snr > 20 ? "#ff9800" : "#f44336"
                            ctx.fillRect(bx, by, barW, barH)

                            // PRN label
                            if (barW > 20) {
                                ctx.fillStyle = "#90caf9"
                                ctx.font = "9px monospace"
                                ctx.fillText("G" + (b + 1), bx + 2, h - 2)
                            }
                        }
                    }
                }
            }

            // ── Satellite List ────────────────────────────
            GroupBox {
                title: "Satellites in View"
                Layout.fillWidth: true
                label: Text {
                    text: "📋 Satellite List (" + GpsManager.satelliteCount + ")"
                    color: "#90caf9"
                    font.pixelSize: 13
                    font.bold: true
                }
                background: Rectangle {
                    color: "#162447"
                    radius: 6
                    border.color: "#1f4068"
                }

                Column {
                    anchors.fill: parent
                    spacing: 2

                    // Header
                    Rectangle {
                        width: parent.width
                        height: 24
                        color: "#0d2137"
                        radius: 4
                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 8; anchors.rightMargin: 8
                            Text { text: "PRN"; color: "#78909c"; font.pixelSize: 11; font.bold: true; width: 50 }
                            Text { text: "Elev"; color: "#78909c"; font.pixelSize: 11; font.bold: true; width: 50 }
                            Text { text: "Azim"; color: "#78909c"; font.pixelSize: 11; font.bold: true; width: 60 }
                            Text { text: "SNR"; color: "#78909c"; font.pixelSize: 11; font.bold: true; width: 50 }
                            Text { text: "Status"; color: "#78909c"; font.pixelSize: 11; font.bold: true }
                        }
                    }

                    // Placeholder rows — real data needs C++ satellite model
                    Repeater {
                        model: GpsManager.satelliteCount

                        Rectangle {
                            width: parent ? parent.width : 0
                            height: 22
                            color: index % 2 === 0 ? "transparent" : "#0d2137"
                            radius: 3

                            Row {
                                anchors.fill: parent
                                anchors.leftMargin: 8; anchors.rightMargin: 8
                                Text { text: "G" + (index + 1); color: "#e0e0e0"; font.pixelSize: 11; width: 50; font.family: "monospace" }
                                Text { text: Math.floor(Math.random() * 85 + 5) + "°"; color: "#b0bec5"; font.pixelSize: 11; width: 50 }
                                Text { text: Math.floor(Math.random() * 360) + "°"; color: "#b0bec5"; font.pixelSize: 11; width: 60 }
                                Text { text: Math.floor(Math.random() * 45 + 5) + " dB"; color: "#b0bec5"; font.pixelSize: 11; width: 50 }
                                Text { text: "OK"; color: "#81c784"; font.pixelSize: 11 }
                            }
                        }
                    }
                }
            }
        }
    }
}

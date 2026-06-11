import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: satellitePanel

    // 缓存卫星数据 — 每次 satelliteInfoChanged 信号只查询一次 C++ 层
    property var satellites: []

    // ── 星座辅助函数 ──────────────────────────────────
    // system: 0=Unknown, 1=GPS, 2=GLONASS, 3=Galileo, 4=BeiDou, 5=QZSS, 6=NavIC
    function systemPrefix(sys) {
        switch (sys) {
            case 1: return "G"   // GPS
            case 2: return "R"   // GLONASS
            case 3: return "E"   // Galileo
            case 4: return "C"   // BeiDou
            case 5: return "J"   // QZSS
            case 6: return "I"   // NavIC
            default: return "S"  // Unknown
        }
    }

    function systemColor(sys) {
        switch (sys) {
            case 1: return "#4fc3f7"  // GPS — 浅蓝
            case 2: return "#f06292"  // GLONASS — 粉红
            case 3: return "#ffd54f"  // Galileo — 琥珀
            case 4: return "#81c784"  // BeiDou — 绿色
            case 5: return "#ba68c8"  // QZSS — 紫色
            case 6: return "#ff8a65"  // NavIC — 橙色
            default: return "#78909c" // Unknown — 灰色
        }
    }

    // ── Satellite data model ──────────────────────────
    ListModel {
        id: satellitesModel
    }

    // ── Refresh when satellite info arrives ───────────
    Connections {
        target: GpsManager
        function onSatelliteInfoChanged() {
            satellites = GpsManager.satellitesInView()
            satellitesModel.clear()
            for (var i = 0; i < satellites.length; i++) {
                satellitesModel.append(satellites[i])
            }
        }
    }

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

                    // Repaint when satellite data changes
                    Connections {
                        target: GpsManager
                        function onSatelliteInfoChanged() {
                            skyPlot.requestPaint()
                        }
                    }

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

                        // Plot real satellites — shape by constellation, color by SNR
                        var sats = satellitePanel.satellites
                        for (var s = 0; s < sats.length; s++) {
                            var sat = sats[s]
                            var satElev = sat.elevation
                            var satAzim = sat.azimuth
                            var azRad = (90 - satAzim) * Math.PI / 180
                            var elevR = r * (1.0 - satElev / 90.0)

                            var sx = cx + elevR * Math.cos(azRad)
                            var sy = cy - elevR * Math.sin(azRad)

                            // Color by SNR quality
                            var snr = sat.snr
                            if (snr > 35)
                                ctx.fillStyle = "#4caf50"
                            else if (snr > 20)
                                ctx.fillStyle = "#ff9800"
                            else if (snr > 0)
                                ctx.fillStyle = "#f44336"
                            else
                                ctx.fillStyle = "#42a5f5"

                            // Shape by constellation
                            var sz = 4
                            ctx.beginPath()
                            var sys = sat.system || 0
                            if (sys === 3) {
                                // Galileo — triangle
                                ctx.moveTo(sx, sy - sz); ctx.lineTo(sx + sz, sy + sz)
                                ctx.lineTo(sx - sz, sy + sz); ctx.closePath()
                            } else if (sys === 2 || sys === 4) {
                                // GLONASS / BeiDou — diamond
                                ctx.moveTo(sx, sy - sz); ctx.lineTo(sx + sz, sy)
                                ctx.lineTo(sx, sy + sz); ctx.lineTo(sx - sz, sy)
                                ctx.closePath()
                            } else {
                                // GPS / QZSS / NavIC / Unknown — circle
                                ctx.arc(sx, sy, sz, 0, 2 * Math.PI)
                            }
                            ctx.fill()

                            // PRN label with constellation prefix
                            ctx.fillStyle = systemColor(sat.system)
                            ctx.font = "8px monospace"
                            var label = systemPrefix(sat.system) + sat.prn
                            ctx.fillText(label, sx + 5, sy + 3)
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

                    // Repaint when satellite data changes
                    Connections {
                        target: GpsManager
                        function onSatelliteInfoChanged() {
                            snrCanvas.requestPaint()
                        }
                    }

                    onPaint: {
                        var ctx = getContext("2d")
                        var w = width
                        var h = height
                        ctx.clearRect(0, 0, w, h)

                        var sats = satellitePanel.satellites
                        var bars = Math.max(sats.length, 4)
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
                            var sat = b < sats.length ? sats[b] : null
                            var snr = sat ? sat.snr : 0
                            var prn = sat ? sat.prn : (b + 1)
                            var barH = h * (snr / maxSnr)
                            var bx = b * (barW + 4)
                            var by = h - barH

                            ctx.fillStyle = snr > 35 ? "#4caf50" :
                                            snr > 20 ? "#ff9800" :
                                            snr > 0  ? "#f44336" : "#37474f"
                            ctx.fillRect(bx, by, barW, barH)

                            // PRN label
                            if (barW > 20) {
                                ctx.fillStyle = "#90caf9"
                                ctx.font = "9px monospace"
                                ctx.fillText("G" + prn, bx + 2, h - 2)
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
                    text: "📋 Satellite List (" + satellitesModel.count + ")"
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

                    // Real satellite rows from C++ model
                    Repeater {
                        model: satellitesModel

                        Rectangle {
                            width: parent ? parent.width : 0
                            height: 22
                            color: index % 2 === 0 ? "transparent" : "#0d2137"
                            radius: 3

                            Row {
                                anchors.fill: parent
                                anchors.leftMargin: 8; anchors.rightMargin: 8
                                Text {
                                    text: systemPrefix(model.system || 0) + model.prn
                                    color: systemColor(model.system || 0)
                                    font.pixelSize: 11; width: 50; font.family: "monospace"
                                    font.bold: true
                                }
                                Text { text: model.elevation + "°"; color: "#b0bec5"; font.pixelSize: 11; width: 50 }
                                Text { text: model.azimuth + "°"; color: "#b0bec5"; font.pixelSize: 11; width: 60 }
                                Text { text: model.snr + " dB"; color: model.snr > 35 ? "#81c784" : model.snr > 20 ? "#ffcc02" : "#ef9a9a"; font.pixelSize: 11; width: 50 }
                                Text { text: model.snr > 0 ? "OK" : "—"; color: model.snr > 0 ? "#81c784" : "#546e7a"; font.pixelSize: 11 }
                            }
                        }
                    }
                }
            }
        }
    }
}

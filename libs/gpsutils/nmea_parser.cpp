#include "nmea_parser.h"
#include <QtMath>

// ────────────────────────────────────────────────────────────
//  NMEAParser 实现
// ────────────────────────────────────────────────────────────

NMEAParser::NMEAParser(QObject *parent)
    : QObject(parent)
{
}

int NMEAParser::feed(const QByteArray &rawData)
{
    m_buffer.append(rawData);
    int count = 0;

    while (true) {
        int idx = m_buffer.indexOf('\n');
        if (idx < 0) break;

        QByteArray line = m_buffer.left(idx).trimmed();
        m_buffer.remove(0, idx + 1);

        if (line.isEmpty() || !line.startsWith('$'))
            continue;

        // 去掉 $ 前缀和 * 校验和部分
        QByteArray sentence = line.mid(1); // skip '$'

        GpsData parsed = m_lastData;  // start from accumulated state
        if (parseSentence(sentence, parsed)) {
            m_lastData   = parsed;
            m_posUpdated = true;
            ++count;
        }
    }

    if (m_posUpdated) {
        m_posUpdated = false;
        emit positionUpdated(m_lastData);
    }

    return count;
}

bool NMEAParser::parseSentence(const QByteArray &sentence, GpsData &outData)
{
    // 分离校验和
    QByteArray body;
    int starIdx = sentence.indexOf('*');
    if (starIdx >= 0) {
        body = sentence.left(starIdx);
        QByteArray checksum = sentence.mid(starIdx + 1);
        // 校验和验证
        Q_UNUSED(checksum);
        if (!validateChecksum(sentence))
            return false;
    } else {
        body = sentence;
    }

    // 按逗号分割
    QList<QByteArray> fields;
    for (const QByteArray &f : body.split(','))
        fields.append(f);

    if (fields.isEmpty())
        return false;

    QString type = QString::fromLatin1(fields[0]);

    // 去掉 Talker ID 前两位, 只保留语句类型
    if (type.size() >= 5)
        type = type.mid(1); // e.g. "GPGGA" → "PGGA"? 实际保留后三位即可
    // 简化: 直接判断关键字
    if (type.endsWith("GGA"))
        return parseGPGGA(body, outData);
    if (type.endsWith("RMC"))
        return parseGPRMC(body, outData);
    if (type.endsWith("VTG"))
        return parseGPVTG(body, outData);
    if (type.endsWith("GSA"))
        return parseGPGSA(body, outData);
    if (type.endsWith("GSV"))
        return parseGPGSV(body, outData);

    return false;
}

bool NMEAParser::validateChecksum(const QByteArray &sentence)
{
    int starIdx = sentence.indexOf('*');
    if (starIdx < 0)
        return true; // 无校验和, 不验证

    QByteArray body = sentence.left(starIdx); // content between start and '*' ($ already stripped by caller)
    QByteArray csHex = sentence.mid(starIdx + 1);

    // XOR all characters between $ and *
    char checksum = 0;
    for (char c : body)
        checksum ^= c;

    bool ok = false;
    int expected = csHex.toInt(&ok, 16);
    return ok && (static_cast<unsigned char>(checksum) == static_cast<unsigned char>(expected));
}

// ────────────────────────────────────────────────────────────
//  $GPGGA — 定位信息
// ────────────────────────────────────────────────────────────
bool NMEAParser::parseGPGGA(const QByteArray &body, GpsData &data)
{
    QList<QByteArray> f;
    for (const QByteArray &s : body.split(','))
        f.append(s);

    if (f.size() < 15)
        return false;

    // UTC 时间 hhmmss.ss
    QByteArray timeStr = f[1];
    if (!timeStr.isEmpty()) {
        QTime t = QTime::fromString(QString::fromLatin1(timeStr.left(6)), "hhmmss");
        if (t.isValid()) {
            data.timestamp.setTime(t);
            data.timestamp.setDate(QDate::currentDate());
        }
    }

    // 纬度 ddmm.mmmm
    if (!f[2].isEmpty() && !f[3].isEmpty()) {
        data.latitude = nmeaToDecimal(f[2].toDouble(), QString::fromLatin1(f[3]));
    }

    // 经度 dddmm.mmmm
    if (!f[4].isEmpty() && !f[5].isEmpty()) {
        data.longitude = nmeaToDecimal(f[4].toDouble(), QString::fromLatin1(f[5]));
    }

    // 定位质量
    if (!f[6].isEmpty())
        data.fixQuality = f[6].toInt();

    // 卫星数量
    if (!f[7].isEmpty())
        data.satelliteCount = f[7].toInt();

    // HDOP
    if (!f[8].isEmpty())
        data.hdop = f[8].toDouble();

    // 海拔高度
    if (!f[9].isEmpty())
        data.altitude = f[9].toDouble();

    if (data.fixQuality > 0 && data.satelliteCount > 0)
        data.isValid = true;

    return true;
}

// ────────────────────────────────────────────────────────────
//  $GPRMC — 推荐最小定位信息
// ────────────────────────────────────────────────────────────
bool NMEAParser::parseGPRMC(const QByteArray &body, GpsData &data)
{
    QList<QByteArray> f;
    for (const QByteArray &s : body.split(','))
        f.append(s);

    if (f.size() < 13)
        return false;

    // UTC 时间
    if (!f[1].isEmpty() && !f[9].isEmpty()) {
        QString timeStr = QString::fromLatin1(f[1]).left(6);
        QString dateStr = QString::fromLatin1(f[9]); // ddmmyy
        QTime t = QTime::fromString(timeStr, "hhmmss");
        QDate d = QDate::fromString(dateStr, "ddMMyy");
        if (t.isValid() && d.isValid())
            data.timestamp = QDateTime(d, t, Qt::UTC).toLocalTime();
    }

    // 状态 A=有效 V=无效
    bool active = (f[2] == "A");

    // 纬度
    if (!f[3].isEmpty() && !f[4].isEmpty())
        data.latitude = nmeaToDecimal(f[3].toDouble(), QString::fromLatin1(f[4]));

    // 经度
    if (!f[5].isEmpty() && !f[6].isEmpty())
        data.longitude = nmeaToDecimal(f[5].toDouble(), QString::fromLatin1(f[6]));

    // 速度 (节)
    if (!f[7].isEmpty()) {
        data.speedKnots = f[7].toDouble();
        data.speedKmh   = data.speedKnots * 1.852;
    }

    // 方位角
    if (!f[8].isEmpty())
        data.course = f[8].toDouble();

    // 磁偏角
    if (!f[10].isEmpty()) {
        data.magVariation = f[10].toDouble();
        if (!f[11].isEmpty() && f[11] == "W")
            data.magVariation = -data.magVariation;
    }

    data.isValid = active && (data.fixQuality > 0 || data.satelliteCount > 0);
    return true;
}

// ────────────────────────────────────────────────────────────
//  $GPVTG — 对地速度和航向
// ────────────────────────────────────────────────────────────
bool NMEAParser::parseGPVTG(const QByteArray &body, GpsData &data)
{
    QList<QByteArray> f;
    for (const QByteArray &s : body.split(','))
        f.append(s);

    if (f.size() < 9)
        return false;

    if (!f[1].isEmpty())
        data.course = f[1].toDouble();

    if (!f[5].isEmpty())
        data.speedKnots = f[5].toDouble();

    if (!f[7].isEmpty())
        data.speedKmh = f[7].toDouble();

    return true;
}

// ────────────────────────────────────────────────────────────
//  $GPGSA — DOP 和活动卫星
// ────────────────────────────────────────────────────────────
bool NMEAParser::parseGPGSA(const QByteArray &body, GpsData &data)
{
    QList<QByteArray> f;
    for (const QByteArray &s : body.split(','))
        f.append(s);

    if (f.size() < 18)
        return false;

    if (!f[15].isEmpty())
        data.pdop = f[15].toDouble();
    if (!f[16].isEmpty())
        data.hdop = f[16].toDouble();
    if (!f[17].isEmpty())
        data.vdop = f[17].trimmed().toDouble();

    return true;
}

// ────────────────────────────────────────────────────────────
//  $GPGSV — 可见卫星
// ────────────────────────────────────────────────────────────
bool NMEAParser::parseGPGSV(const QByteArray &body, GpsData &data)
{
    QList<QByteArray> f;
    for (const QByteArray &s : body.split(','))
        f.append(s);

    if (f.size() < 4)
        return false;

    int totalMessages   = f[1].toInt();
    int messageNumber   = f[2].toInt();
    int satellitesInView = f[3].toInt();

    data.satelliteCount = satellitesInView;

    // 每条消息可包含 4 颗卫星
    int fieldIdx = 4;
    while (fieldIdx + 3 < f.size()) {
        GpsData::SatelliteInfo sat;
        sat.prn       = f[fieldIdx].toInt();
        sat.elevation = f[fieldIdx + 1].toInt();
        sat.azimuth   = f[fieldIdx + 2].toInt();
        sat.snr       = f[fieldIdx + 3].toInt();
        data.satellitesInView.append(sat);
        fieldIdx += 4;
    }

    return true;
}

// ────────────────────────────────────────────────────────────
//  辅助: NMEA 坐标 → 十进制度
// ────────────────────────────────────────────────────────────
double NMEAParser::nmeaToDecimal(double nmeaCoord, const QString &hemisphere)
{
    // ddmm.mmmm → dd + mm.mmmm/60
    int    deg = static_cast<int>(nmeaCoord / 100.0);
    double min = nmeaCoord - deg * 100.0;
    double dec = deg + min / 60.0;

    if (hemisphere == "S" || hemisphere == "W")
        dec = -dec;

    return dec;
}

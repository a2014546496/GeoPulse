#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QDateTime>
#include <QList>

// ──────────────────────────────────────
//  卫星信息（独立于定位数据，避免每次 GpsData 复制携带完整卫星列表）
// ──────────────────────────────────────
struct SatelliteInfo {
    enum System {
        Unknown = 0,
        GPS,
        GLONASS,
        Galileo,
        BeiDou,
        QZSS,
        NavIC
    };

    int    prn       = 0;
    int    elevation = 0;   // 仰角 (度)
    int    azimuth   = 0;   // 方位角 (度)
    int    snr       = 0;   // 信噪比 (dB)
    System system    = Unknown;

    bool operator==(const SatelliteInfo &other) const {
        return prn == other.prn
            && elevation == other.elevation
            && azimuth == other.azimuth
            && snr == other.snr
            && system == other.system;
    }
};

// ──────────────────────────────────────
//  结构化 GPS 数据
// ──────────────────────────────────────
struct GpsData {
    QDateTime timestamp;
    double latitude    = 0.0;   // 十进制度 (正=北, 负=南)
    double longitude   = 0.0;   // 十进制度 (正=东, 负=西)
    double altitude    = 0.0;   // 米
    double speedKnots  = 0.0;   // 节
    double speedKmh    = 0.0;   // km/h
    double course      = 0.0;   // 方位角 (度, 从真北顺时针)
    int    fixQuality  = 0;     // 0=无效, 1=GPS, 2=DGPS, 4=RTK固定, 5=RTK浮动
    int    satelliteCount = 0;
    double hdop        = 99.99; // 水平精度因子
    double pdop        = 99.99;
    double vdop        = 99.99;
    double magVariation = 0.0;  // 磁偏角
    bool   isValid     = false;
};

// ──────────────────────────────────────
//  NMEA 0183 解析器
// ──────────────────────────────────────
class NMEAParser : public QObject
{
    Q_OBJECT

public:
    explicit NMEAParser(QObject *parent = nullptr);

    /// 喂入原始字节流，按行分割并解析
    /// @return 新解析出的 GpsData 条数
    int feed(const QByteArray &rawData);

    /// 解析单条 NMEA 语句 (不含 $ 前缀) —— 向后兼容
    static bool parseSentence(const QByteArray &sentence, GpsData &outData);

    /// 解析单条 NMEA 语句，同时输出卫星信息
    static bool parseSentence(const QByteArray &sentence,
                              GpsData &outData,
                              QList<SatelliteInfo> &outSatellites);

    /// 获取当前累积的卫星列表
    const QList<SatelliteInfo> &satellitesInView() const { return m_satellitesInView; }

signals:
    /// 解析出有效定位数据
    void positionUpdated(const GpsData &data);
    /// 原始 NMEA 语句 (供调试/日志)
    void rawSentence(const QString &sentence);

private:
    static bool validateChecksum(const QByteArray &sentence);

    // 各 NMEA 语句解析
    static bool parseGPGGA(const QByteArray &fields, GpsData &data);
    static bool parseGPRMC(const QByteArray &fields, GpsData &data);
    static bool parseGPVTG(const QByteArray &fields, GpsData &data);
    static bool parseGPGSA(const QByteArray &fields, GpsData &data);
    static bool parseGPGSV(const QByteArray &fields, GpsData &data,
                           QList<SatelliteInfo> &outSatellites,
                           SatelliteInfo::System constellation);

    static double nmeaToDecimal(double nmeaCoord, const QString &hemisphere);
    static SatelliteInfo::System constellationFromTalkerId(const QString &talker);

    QByteArray m_buffer;               // 行缓冲
    GpsData    m_lastData;             // 累积定位数据 (多语句合并)
    QList<SatelliteInfo> m_satellitesInView;  // 累积卫星列表 (跨 GSV 语句)
    bool       m_posUpdated = false;
};

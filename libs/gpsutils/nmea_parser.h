#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QDateTime>
#include <QList>

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

    // 汇总卫星信息 (GPGSA / GPGSV)
    struct SatelliteInfo {
        int    prn       = 0;
        int    elevation = 0;   // 仰角 (度)
        int    azimuth   = 0;   // 方位角 (度)
        int    snr       = 0;   // 信噪比 (dB)
    };
    QList<SatelliteInfo> satellitesInView;
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

    /// 解析单条 NMEA 语句 (不含 $ 前缀)
    static bool parseSentence(const QByteArray &sentence, GpsData &outData);

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
    static bool parseGPGSV(const QByteArray &fields, GpsData &data);

    static double nmeaToDecimal(double nmeaCoord, const QString &hemisphere);

    QByteArray m_buffer;        // 行缓冲
    GpsData    m_lastData;       // 累积数据 (多语句合并)
    QList<GpsData::SatelliteInfo> m_pendingSatellites;
    bool       m_posUpdated = false;
};

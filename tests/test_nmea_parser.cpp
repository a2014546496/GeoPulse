#include "nmea_parser.h"

#include <QTest>
#include <QSignalSpy>

class TestNMEAParser : public QObject
{
    Q_OBJECT

private slots:
    void init()
    {
        m_parser = new NMEAParser(this);
    }

    void cleanup()
    {
        delete m_parser;
        m_parser = nullptr;
    }

    // ── GGA sentence parsing ──────────────────────────
    void testParseGGA()
    {
        // $GPGGA,065520.000,3990.8342,N,11623.5225,E,1,08,1.2,52.3,M,-10.5,M,,*47
        QByteArray sentence = "GPGGA,065520.000,3990.8342,N,11623.5225,E,1,08,1.2,52.3,M,-10.5,M,,*47";

        GpsData data;
        bool ok = NMEAParser::parseSentence(sentence, data);

        QVERIFY(ok);
        QVERIFY(data.isValid);
        QCOMPARE(data.fixQuality, 1);
        QCOMPARE(data.satelliteCount, 8);
        QCOMPARE(data.hdop, 1.2);
        QCOMPARE(data.altitude, 52.3);

        // Latitude: ddmm.mmmm = 3990.8342 → 39° + 90.8342'/60
        double expectedLat = 39.0 + 90.8342 / 60.0;
        QVERIFY(qAbs(data.latitude - expectedLat) < 0.0001);

        // Longitude: dddmm.mmmm = 11623.5225 → 116° + 23.5225'/60
        double expectedLon = 116.0 + 23.5225 / 60.0;
        QVERIFY(qAbs(data.longitude - expectedLon) < 0.0001);
    }

    // ── RMC sentence parsing ──────────────────────────
    void testParseRMC()
    {
        // $GPRMC,065520.000,A,3990.8342,N,11623.5225,E,0.5,45.2,090626,6.5,W,A*24
        QByteArray sentence = "GPRMC,065520.000,A,3990.8342,N,11623.5225,E,0.5,45.2,090626,6.5,W,A*24";

        GpsData data;
        bool ok = NMEAParser::parseSentence(sentence, data);

        QVERIFY(ok);
        // Standalone RMC: fixQuality/satelliteCount are defaults (0),
        // so isValid evaluates to false (needs GGA cross-data).
        // RMC "A" status alone is not enough per our logic.
        QVERIFY(!data.isValid);
        QCOMPARE(data.speedKnots, 0.5);
        QVERIFY(qAbs(data.speedKmh - 0.926) < 0.01); // 0.5 * 1.852
        QCOMPARE(data.course, 45.2);
        QVERIFY(qAbs(data.magVariation + 6.5) < 0.01); // W → negative
    }

    // ── VTG sentence parsing ──────────────────────────
    void testParseVTG()
    {
        QByteArray sentence = "GPVTG,50.5,T,57.0,M,0.0021,N,0.0039,K,A*28";

        GpsData data;
        bool ok = NMEAParser::parseSentence(sentence, data);

        QVERIFY(ok);
        QCOMPARE(data.course, 50.5);
        QCOMPARE(data.speedKnots, 0.0021);
        QCOMPARE(data.speedKmh, 0.0039);
    }

    // ── GSA sentence parsing ──────────────────────────
    void testParseGSA()
    {
        QByteArray sentence = "GPGSA,A,3,01,03,07,08,11,14,17,22,,,,,1.5,1.2,0.9*32";

        GpsData data;
        bool ok = NMEAParser::parseSentence(sentence, data);

        QVERIFY(ok);
        QCOMPARE(data.pdop, 1.5);
        QCOMPARE(data.hdop, 1.2);
        QCOMPARE(data.vdop, 0.9);
    }

    // ── GSV sentence parsing ──────────────────────────
    void testParseGSV()
    {
        // $GPGSV,3,1,12,01,45,120,42,03,60,240,45,07,15,030,38,08,75,300,48*71
        QByteArray sentence = "GPGSV,3,1,12,01,45,120,42,03,60,240,45,07,15,030,38,08,75,300,48*71";

        GpsData data;
        bool ok = NMEAParser::parseSentence(sentence, data);

        QVERIFY(ok);
        QCOMPARE(data.satelliteCount, 12);
        QCOMPARE(data.satellitesInView.size(), 4);

        // Check first satellite
        QCOMPARE(data.satellitesInView[0].prn, 1);
        QCOMPARE(data.satellitesInView[0].elevation, 45);
        QCOMPARE(data.satellitesInView[0].azimuth, 120);
        QCOMPARE(data.satellitesInView[0].snr, 42);
    }

    // ── Checksum validation ───────────────────────────
    void testInvalidChecksum()
    {
        // Deliberately wrong checksum (correct is *47)
        QByteArray sentence = "GPGGA,065520.000,3990.8342,N,11623.5225,E,1,08,1.2,52.3,M,-10.5,M,,*00";

        GpsData data;
        bool ok = NMEAParser::parseSentence(sentence, data);

        QVERIFY(!ok);
    }

    // ── Feed (buffered input) ─────────────────────────
    void testFeed()
    {
        QSignalSpy spy(m_parser, &NMEAParser::positionUpdated);

        QByteArray partial = "$GPGGA,065520.000,3990.8342,N,11623.5225,E,1,08,1.2,52.3,M,-10.5,M,,*47\r\n";
        int count = m_parser->feed(partial);

        QCOMPARE(count, 1);
        QCOMPARE(spy.count(), 1);
    }

    // ── Handle multiple sentences in one chunk ────────
    void testMultipleSentences()
    {
        QSignalSpy spy(m_parser, &NMEAParser::positionUpdated);

        QByteArray data =
            "$GPGGA,065520.000,3990.8342,N,11623.5225,E,1,08,1.2,52.3,M,-10.5,M,,*47\r\n"
            "$GPRMC,065520.000,A,3990.8342,N,11623.5225,E,0.5,45.2,090626,6.5,W,A*24\r\n";

        int count = m_parser->feed(data);

        QCOMPARE(count, 2);
        QCOMPARE(spy.count(), 1); // Only one emission per feed batch
    }

private:
    NMEAParser *m_parser = nullptr;
};

QTEST_MAIN(TestNMEAParser)
#include "test_nmea_parser.moc"

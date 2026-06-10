#include "geo_utils.h"

#include <QTest>
#include <QtMath>

class TestGeoUtils : public QObject
{
    Q_OBJECT

private slots:
    // ── Haversine distance ────────────────────────────
    void testDistance()
    {
        // Paris → London: ~343 km
        QGeoCoordinate paris(48.8566, 2.3522);
        QGeoCoordinate london(51.5074, -0.1278);

        double dist = GeoUtils::distance(paris, london);
        // Expected: ~343,000 meters
        QVERIFY(qAbs(dist - 343000.0) < 10000.0); // within 10 km tolerance
    }

    void testDistanceSamePoint()
    {
        QGeoCoordinate a(39.9042, 116.4074);
        double dist = GeoUtils::distance(a, a);
        QCOMPARE(dist, 0.0);
    }

    void testDistanceAntipodal()
    {
        // Approximately opposite sides
        QGeoCoordinate a(40.0, 116.0);
        QGeoCoordinate b(-40.0, -64.0); // ~180° longitude apart

        double dist = GeoUtils::distance(a, b);
        // Should be very large, around half circumference
        QVERIFY(dist > 15000000.0); // > 15,000 km
        QVERIFY(dist < 22000000.0); // < 22,000 km (max ~20,037)
    }

    // ── Bearing ───────────────────────────────────────
    void testBearingNorth()
    {
        QGeoCoordinate a(39.0, 116.0);
        QGeoCoordinate b(40.0, 116.0); // due north

        double brng = GeoUtils::bearing(a, b);
        // Should be close to 0° (or 360°)
        QVERIFY(brng < 1.0 || brng > 359.0);
    }

    void testBearingEast()
    {
        QGeoCoordinate a(39.0, 116.0);
        QGeoCoordinate b(39.0, 117.0); // due east

        double brng = GeoUtils::bearing(a, b);
        QVERIFY(qAbs(brng - 90.0) < 1.0);
    }

    void testBearingRange()
    {
        QGeoCoordinate a(39.0, 116.0);
        QGeoCoordinate b(40.0, 117.0);

        double brng = GeoUtils::bearing(a, b);
        QVERIFY(brng >= 0.0);
        QVERIFY(brng < 360.0);
    }

    // ── Destination ───────────────────────────────────
    void testDestinationRoundtrip()
    {
        QGeoCoordinate origin(39.0, 116.0);
        double dist = 100000.0; // 100 km
        double brng = 45.0;

        QGeoCoordinate dest = GeoUtils::destination(origin, brng, dist);

        // Distance back should be ~100 km
        double backDist = GeoUtils::distance(origin, dest);
        QVERIFY(qAbs(backDist - dist) < 10.0); // Within 10 meters

        // Bearing back should be ~225°
        double backBrng = GeoUtils::bearing(dest, origin);
        QVERIFY(qAbs(backBrng - (brng + 180.0)) < 1.0 ||
                qAbs(backBrng - (brng - 180.0)) < 1.0);
    }

    // ── Point in polygon ──────────────────────────────
    void testPointInPolygon()
    {
        // Simple square around Beijing
        QList<QGeoCoordinate> square;
        square << QGeoCoordinate(40.0, 116.0)
               << QGeoCoordinate(40.0, 117.0)
               << QGeoCoordinate(39.0, 117.0)
               << QGeoCoordinate(39.0, 116.0);

        QGeoCoordinate inside(39.5, 116.5);
        QGeoCoordinate outside(41.0, 116.5);

        QVERIFY(GeoUtils::pointInPolygon(inside, square));
        QVERIFY(!GeoUtils::pointInPolygon(outside, square));
    }

    void testPointInTriangle()
    {
        QList<QGeoCoordinate> triangle;
        triangle << QGeoCoordinate(40.0, 116.0)
                 << QGeoCoordinate(40.0, 117.0)
                 << QGeoCoordinate(39.0, 116.5);

        QGeoCoordinate inside(39.8, 116.5);
        QGeoCoordinate outside(40.5, 116.5);

        QVERIFY(GeoUtils::pointInPolygon(inside, triangle));
        QVERIFY(!GeoUtils::pointInPolygon(outside, triangle));
    }

    void testInsufficientPolygon()
    {
        QList<QGeoCoordinate> line;
        line << QGeoCoordinate(40.0, 116.0)
             << QGeoCoordinate(40.0, 117.0);

        QGeoCoordinate pt(40.0, 116.5);
        QVERIFY(!GeoUtils::pointInPolygon(pt, line));
    }

    // ── Polygon area ──────────────────────────────────
    void testPolygonArea()
    {
        // 1°×1° square at equator ≈ 123.6 km × 111.3 km ≈ 13,750 km²
        QList<QGeoCoordinate> square;
        square << QGeoCoordinate(0.0, 0.0)
               << QGeoCoordinate(1.0, 0.0)
               << QGeoCoordinate(1.0, 1.0)
               << QGeoCoordinate(0.0, 1.0);

        double area = GeoUtils::polygonArea(square);
        // Rough estimate: ~12,300 km²
        QVERIFY(area > 1e10);  // > 10,000 km²
        QVERIFY(area < 2e10);  // < 20,000 km²
    }

    void testPolygonAreaTooSmall()
    {
        QList<QGeoCoordinate> line;
        line << QGeoCoordinate(40.0, 116.0)
             << QGeoCoordinate(40.0, 117.0);

        double area = GeoUtils::polygonArea(line);
        QCOMPARE(area, 0.0);
    }

    // ── Track simplification ──────────────────────────
    void testSimplifyPreservesEndpoints()
    {
        QList<QGeoCoordinate> points;
        points << QGeoCoordinate(39.0, 116.0)
               << QGeoCoordinate(39.01, 116.01)
               << QGeoCoordinate(39.02, 116.02)
               << QGeoCoordinate(39.1, 116.1);

        QList<QGeoCoordinate> simplified = GeoUtils::simplifyTrack(points, 100.0);
        QVERIFY(simplified.size() >= 2);
        QCOMPARE(simplified.first().latitude(), points.first().latitude());
        QCOMPARE(simplified.last().latitude(), points.last().latitude());
    }

    void testSimplifyShortTrack()
    {
        // Less than 3 points should be returned as-is
        QList<QGeoCoordinate> points;
        points << QGeoCoordinate(39.0, 116.0)
               << QGeoCoordinate(39.1, 116.1);

        QList<QGeoCoordinate> simplified = GeoUtils::simplifyTrack(points);
        QCOMPARE(simplified.size(), 2);
    }
};

QTEST_MAIN(TestGeoUtils)
#include "test_geo_utils.moc"

#include "geo_utils.h"
#include <QtMath>
#include <algorithm>

namespace GeoUtils {

static constexpr double EARTH_RADIUS_M = 6371000.0;

double distance(const QGeoCoordinate &a, const QGeoCoordinate &b)
{
    double lat1 = qDegreesToRadians(a.latitude());
    double lat2 = qDegreesToRadians(b.latitude());
    double dLat = qDegreesToRadians(b.latitude() - a.latitude());
    double dLon = qDegreesToRadians(b.longitude() - a.longitude());

    double sinLat = sin(dLat / 2.0);
    double sinLon = sin(dLon / 2.0);

    double aVal = sinLat * sinLat + cos(lat1) * cos(lat2) * sinLon * sinLon;
    double c    = 2.0 * atan2(sqrt(aVal), sqrt(1.0 - aVal));

    return EARTH_RADIUS_M * c;
}

double bearing(const QGeoCoordinate &a, const QGeoCoordinate &b)
{
    double lat1 = qDegreesToRadians(a.latitude());
    double lat2 = qDegreesToRadians(b.latitude());
    double dLon = qDegreesToRadians(b.longitude() - a.longitude());

    double y = sin(dLon) * cos(lat2);
    double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon);

    double brng = qRadiansToDegrees(atan2(y, x));
    return fmod(brng + 360.0, 360.0);
}

QGeoCoordinate destination(const QGeoCoordinate &origin, double bearingDeg,
                            double distanceMeters)
{
    double lat1   = qDegreesToRadians(origin.latitude());
    double lon1   = qDegreesToRadians(origin.longitude());
    double brng   = qDegreesToRadians(bearingDeg);
    double angDist = distanceMeters / EARTH_RADIUS_M;

    double lat2 = asin(sin(lat1) * cos(angDist) +
                       cos(lat1) * sin(angDist) * cos(brng));

    double lon2 = lon1 + atan2(sin(brng) * sin(angDist) * cos(lat1),
                                cos(angDist) - sin(lat1) * sin(lat2));

    return QGeoCoordinate(qRadiansToDegrees(lat2), qRadiansToDegrees(lon2));
}

bool pointInPolygon(const QGeoCoordinate &point,
                    const QList<QGeoCoordinate> &polygon)
{
    if (polygon.size() < 3)
        return false;

    double px = point.longitude();
    double py = point.latitude();

    bool inside = false;
    int  n = polygon.size();

    for (int i = 0, j = n - 1; i < n; j = i++) {
        double xi = polygon[i].longitude();
        double yi = polygon[i].latitude();
        double xj = polygon[j].longitude();
        double yj = polygon[j].latitude();

        bool intersect = ((yi > py) != (yj > py)) &&
                         (px < (xj - xi) * (py - yi) / (yj - yi) + xi);
        if (intersect)
            inside = !inside;
    }

    return inside;
}

double polygonArea(const QList<QGeoCoordinate> &polygon)
{
    if (polygon.size() < 3)
        return 0.0;

    double area = 0.0;
    int n = polygon.size();

    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        area += qDegreesToRadians(polygon[j].longitude() - polygon[i].longitude()) *
                (2.0 + sin(qDegreesToRadians(polygon[i].latitude())) +
                       sin(qDegreesToRadians(polygon[j].latitude())));
    }

    area = qAbs(area * EARTH_RADIUS_M * EARTH_RADIUS_M / 2.0);
    return area;
}

// ────────────────────────────────────────────────────────────
//  Douglas-Peucker 抽稀
// ────────────────────────────────────────────────────────────
static void dpRecursive(const QList<QGeoCoordinate> &points, int start, int end,
                        double epsilon, QList<bool> &keep)
{
    if (end <= start + 1)
        return;

    double maxDist = 0.0;
    int    maxIdx  = start;

    QGeoCoordinate p1 = points[start];
    QGeoCoordinate p2 = points[end];

    for (int i = start + 1; i < end; ++i) {
        // 点到线段的大圆距离近似
        double d1 = distance(p1, points[i]);
        double d2 = distance(points[i], p2);
        double dSeg = distance(p1, p2);
        double s = (d1 + d2 + dSeg) / 2.0;
        // 用平面三角形面积 / 底边 来近似
        double area = qMax(0.0, s * (s - d1) * (s - d2) * (s - dSeg));
        double h = (dSeg > 0.001) ? (2.0 * sqrt(area) / dSeg) : 0.0;

        if (h > maxDist) {
            maxDist = h;
            maxIdx  = i;
        }
    }

    if (maxDist > epsilon) {
        keep[maxIdx] = true;
        dpRecursive(points, start, maxIdx, epsilon, keep);
        dpRecursive(points, maxIdx, end, epsilon, keep);
    }
}

QList<QGeoCoordinate> simplifyTrack(const QList<QGeoCoordinate> &points,
                                     double epsilonMeters)
{
    if (points.size() <= 2)
        return points;

    QList<bool> keep(points.size(), false);
    keep.first() = true;
    keep.last()  = true;

    dpRecursive(points, 0, points.size() - 1, epsilonMeters, keep);

    QList<QGeoCoordinate> result;
    for (int i = 0; i < points.size(); ++i) {
        if (keep[i])
            result.append(points[i]);
    }
    return result;
}

} // namespace GeoUtils

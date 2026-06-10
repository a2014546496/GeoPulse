#pragma once

#include <QGeoCoordinate>
#include <QList>

/**
 * @brief 地理计算工具集
 *
 *  - Haversine 距离计算
 *  - 方位角计算
 *  - 点是否在多边形内 (射线法, 用于电子围栏判定)
 *  - 坐标格式转换
 */
namespace GeoUtils {

/// Haversine 球面距离 (米)
double distance(const QGeoCoordinate &a, const QGeoCoordinate &b);

/// 从 a 到 b 的方位角 (度, 0-360, 从真北顺时针)
double bearing(const QGeoCoordinate &a, const QGeoCoordinate &b);

/// 已知起点、方位角、距离(米), 计算终点坐标
QGeoCoordinate destination(const QGeoCoordinate &origin, double bearingDeg, double distanceMeters);

/// 点是否在多边形内部 (射线法)
bool pointInPolygon(const QGeoCoordinate &point,
                    const QList<QGeoCoordinate> &polygon);

/// 多边形面积 (平方米, 球面近似)
double polygonArea(const QList<QGeoCoordinate> &polygon);

/// 轨道平滑抽稀 (Douglas-Peucker, 减少轨迹点)
QList<QGeoCoordinate> simplifyTrack(const QList<QGeoCoordinate> &points,
                                     double epsilonMeters = 5.0);

} // namespace GeoUtils

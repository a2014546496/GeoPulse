#pragma once

#include "gps_manager.h"
#include "track_recorder.h"
#include "geofence_manager.h"

#include <QObject>
#include <QGuiApplication>

/**
 * @brief Application root object — owns core managers and wires them together.
 *
 * Exposed to QML as context property "App" for global access.
 */
class GeoPulseApp : public QObject
{
    Q_OBJECT

    Q_PROPERTY(GpsManager* gpsManager READ gpsManager CONSTANT)
    Q_PROPERTY(TrackRecorder* trackRecorder READ trackRecorder CONSTANT)
    Q_PROPERTY(GeofenceManager* geofenceManager READ geofenceManager CONSTANT)
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)

public:
    explicit GeoPulseApp(QObject *parent = nullptr);
    ~GeoPulseApp() override;

    /// Load config, restore state
    bool initialize();

    /// Save state before exit
    void shutdown();

    GpsManager* gpsManager() const;
    TrackRecorder* trackRecorder() const;
    GeofenceManager* geofenceManager() const;
    QString appVersion() const;

private:
    void wireConnections();
    void loadConfig();
    void saveConfig();

    GpsManager       *m_gpsManager       = nullptr;
    TrackRecorder    *m_trackRecorder    = nullptr;
    GeofenceManager  *m_geofenceManager  = nullptr;
};

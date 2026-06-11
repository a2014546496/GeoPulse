#include "geopulse_app.h"
#include "config_manager.h"
#include "logger.h"

#include <QStandardPaths>
#include <QDir>

GeoPulseApp::GeoPulseApp(QObject *parent)
    : QObject(parent)
    , m_gpsManager(new GpsManager(this))
    , m_trackRecorder(new TrackRecorder(this))
    , m_geofenceManager(new GeofenceManager(this))
{
    wireConnections();
}

GeoPulseApp::~GeoPulseApp()
{
    shutdown();
}

bool GeoPulseApp::initialize()
{
    // Set up logging
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(logPath);
    Logger::instance()->setLogFile(logPath + "/geopulse.log");
    Logger::instance()->info("GeoPulse starting...");

    // Load configuration
    loadConfig();

    return true;
}

void GeoPulseApp::shutdown()
{
    saveConfig();
    Logger::instance()->info("GeoPulse shutting down.");
}

void GeoPulseApp::connectToSource(const QString &address,
                                   const QString &type,
                                   int baudRate)
{
    m_lastAddress = address;
    m_lastBaudRate = baudRate;

    // Persist connection params immediately
    auto *cfg = ConfigManager::instance();
    cfg->setValue("connection.type", type);
    cfg->setValue("connection.address", address);
    cfg->setValue("connection.baudRate", baudRate);

    m_gpsManager->connectToSource(address, type, baudRate);
}

GpsManager* GeoPulseApp::gpsManager() const
{
    return m_gpsManager;
}

TrackRecorder* GeoPulseApp::trackRecorder() const
{
    return m_trackRecorder;
}

GeofenceManager* GeoPulseApp::geofenceManager() const
{
    return m_geofenceManager;
}

QString GeoPulseApp::appVersion() const
{
    return "1.0.0";
}

void GeoPulseApp::wireConnections()
{
    // When a new GPS position arrives, add to track recorder
    connect(m_gpsManager, &GpsManager::newPosition,
            m_trackRecorder, &TrackRecorder::addPoint);

    // When a new GPS position arrives, check geofences
    connect(m_gpsManager, &GpsManager::newPosition,
            this, [this](const GpsData &data) {
                if (data.isValid) {
                    m_geofenceManager->checkPosition(data.latitude, data.longitude);
                }
            });
}

void GeoPulseApp::loadConfig()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + "/config.json";
    auto *cfg = ConfigManager::instance();

    if (!cfg->load(configPath)) {
        Logger::instance()->info("No config file found, using defaults");
        return;
    }

    // Restore last connection settings
    QString lastType = cfg->value("connection.type").toString();
    QString lastAddress = cfg->value("connection.address").toString();
    int lastBaud = cfg->value("connection.baudRate", 9600).toInt();

    if (!lastType.isEmpty() && !lastAddress.isEmpty()) {
        m_lastAddress = lastAddress;
        m_lastBaudRate = lastBaud;
        Logger::instance()->info(QString("Restoring connection: %1 @ %2")
                                     .arg(lastType, lastAddress));
        m_gpsManager->connectToSource(lastAddress, lastType, lastBaud);
    }
}

void GeoPulseApp::saveConfig()
{
    auto *cfg = ConfigManager::instance();

    // Save connection settings for next launch
    if (m_gpsManager->isConnected()) {
        cfg->setValue("connection.type", m_gpsManager->driverType());
        cfg->setValue("connection.address", m_lastAddress);
        cfg->setValue("connection.baudRate", m_lastBaudRate);
    }

    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + "/config.json";
    QDir().mkpath(QFileInfo(configPath).absolutePath());
    cfg->save(configPath);
}

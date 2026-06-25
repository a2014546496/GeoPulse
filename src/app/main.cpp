#include "geopulse_app.h"
#include "logger.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>

int main(int argc, char *argv[])
{

    QGuiApplication app(argc, argv);
    app.setApplicationName("GeoPulse");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("GeoPulse");
    app.setOrganizationDomain("geopulse.local");

    // ── Create application root ──────────────────────────
    GeoPulseApp geoPulseApp;
    if (!geoPulseApp.initialize()) {
        Logger::instance()->error("Failed to initialize GeoPulse");
        return 1;
    }

    // ── QML Engine ───────────────────────────────────────
    QQmlApplicationEngine engine;

    // Expose C++ objects to QML
    QQmlContext *ctx = engine.rootContext();
    ctx->setContextProperty("App", &geoPulseApp);
    ctx->setContextProperty("GpsManager", geoPulseApp.gpsManager());
    ctx->setContextProperty("TrackRecorder", geoPulseApp.trackRecorder());
    ctx->setContextProperty("GeofenceManager", geoPulseApp.geofenceManager());
    ctx->setContextProperty("AlertManager", geoPulseApp.alertManager());
    ctx->setContextProperty("Logger", Logger::instance());

    // ── Load main QML ────────────────────────────────────
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);

    int ret = app.exec();

    // ── Cleanup ──────────────────────────────────────────
    geoPulseApp.shutdown();

    return ret;
}

#include "config_manager.h"

#include <QTest>
#include <QFile>
#include <QTemporaryFile>
#include <QJsonDocument>
#include <QJsonObject>

class TestConfigManager : public QObject
{
    Q_OBJECT

private slots:
    void init()
    {
        m_cfg = ConfigManager::instance();
    }

    // ── Set / Get ─────────────────────────────────────
    void testSetAndGet()
    {
        m_cfg->setValue("test.key", 42);
        QVariant val = m_cfg->value("test.key");
        QCOMPARE(val.toInt(), 42);
    }

    void testDefaultValue()
    {
        QVariant val = m_cfg->value("nonexistent.key", "default_value");
        QCOMPARE(val.toString(), QString("default_value"));
    }

    // ── Nested keys ───────────────────────────────────
    void testNestedKeys()
    {
        m_cfg->setValue("serial.baudrate", 9600);
        m_cfg->setValue("serial.port", "/dev/ttyUSB0");
        m_cfg->setValue("serial.enabled", true);

        QCOMPARE(m_cfg->value("serial.baudrate").toInt(), 9600);
        QCOMPARE(m_cfg->value("serial.port").toString(), QString("/dev/ttyUSB0"));
        QCOMPARE(m_cfg->value("serial.enabled").toBool(), true);
    }

    // ── Save / Load round-trip ────────────────────────
    void testSaveLoadRoundtrip()
    {
        m_cfg->setValue("roundtrip.name", "GeoPulse");
        m_cfg->setValue("roundtrip.version", 2);
        m_cfg->setValue("roundtrip.nested.key", "value");

        // Save to temp file
        QString tempPath = QDir::tempPath() + "/geopulse_test_config.json";
        bool saved = m_cfg->save(tempPath);
        QVERIFY(saved);

        // Clear in-memory data
        m_cfg->setValue("roundtrip", QVariantMap());

        // Load back
        bool loaded = m_cfg->load(tempPath);
        QVERIFY(loaded);

        QCOMPARE(m_cfg->value("roundtrip.name").toString(), QString("GeoPulse"));
        QCOMPARE(m_cfg->value("roundtrip.version").toInt(), 2);
        QCOMPARE(m_cfg->value("roundtrip.nested.key").toString(), QString("value"));

        // Cleanup
        QFile::remove(tempPath);
    }

    // ── Load invalid file ─────────────────────────────
    void testLoadInvalidJson()
    {
        QString tempPath = QDir::tempPath() + "/geopulse_invalid.json";
        QFile f(tempPath);
        f.open(QIODevice::WriteOnly);
        f.write("this is not valid json {{{");
        f.close();

        bool loaded = m_cfg->load(tempPath);
        QVERIFY(!loaded);

        QFile::remove(tempPath);
    }

    // ── Load non-existent file ────────────────────────
    void testLoadNonExistentFile()
    {
        bool loaded = m_cfg->load("/tmp/does_not_exist_config_12345.json");
        QVERIFY(!loaded);
    }

private:
    ConfigManager *m_cfg = nullptr;
};

QTEST_MAIN(TestConfigManager)
#include "test_config_manager.moc"

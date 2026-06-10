#include "driver_factory.h"
#include "serial_driver.h"
#include "tcp_driver.h"
#include "udp_driver.h"

#include <QTest>

class TestDriverFactory : public QObject
{
    Q_OBJECT

private slots:
    // ── Enum-based creation ───────────────────────────
    void testCreateTcp()
    {
        auto driver = DriverFactory::create(DriverFactory::Tcp);
        QVERIFY(driver != nullptr);
        QCOMPARE(driver->driverType(), QString("TCP"));
    }

    void testCreateUdp()
    {
        auto driver = DriverFactory::create(DriverFactory::Udp);
        QVERIFY(driver != nullptr);
        QCOMPARE(driver->driverType(), QString("UDP"));
    }

    void testCreateSerial()
    {
        auto driver = DriverFactory::create(DriverFactory::Serial);
        QVERIFY(driver != nullptr);
        QCOMPARE(driver->driverType(), QString("Serial"));
    }

    // ── String-based creation ─────────────────────────
    void testCreateFromString()
    {
        auto tcp = DriverFactory::create("tcp");
        QVERIFY(tcp != nullptr);
        QCOMPARE(tcp->driverType(), QString("TCP"));

        auto udp = DriverFactory::create("UDP");
        QVERIFY(udp != nullptr);
        QCOMPARE(udp->driverType(), QString("UDP"));

        auto ser = DriverFactory::create("Serial");
        QVERIFY(ser != nullptr);
        QCOMPARE(ser->driverType(), QString("Serial"));
    }

    void testCreateCaseInsensitive()
    {
        auto tcp = DriverFactory::create("TCP");
        QVERIFY(tcp != nullptr);
        QCOMPARE(tcp->driverType(), QString("TCP"));

        auto udp = DriverFactory::create("udp");
        QVERIFY(udp != nullptr);
        QCOMPARE(udp->driverType(), QString("UDP"));
    }

    void testCreateUnknownType()
    {
        auto driver = DriverFactory::create("InvalidType");
        QVERIFY(driver == nullptr);
    }

    void testCreateEmptyString()
    {
        auto driver = DriverFactory::create("");
        QVERIFY(driver == nullptr);
    }

    // ── Available types ───────────────────────────────
    void testAvailableTypes()
    {
        QStringList types = DriverFactory::availableTypes();
        QCOMPARE(types.size(), 3);
        QVERIFY(types.contains("TCP"));
        QVERIFY(types.contains("UDP"));
        QVERIFY(types.contains("Serial"));
    }

    // ── Driver state after construction ───────────────
    void testDriverNotOpenByDefault()
    {
        auto tcp = DriverFactory::create(DriverFactory::Tcp);
        QVERIFY(!tcp->isOpen());

        auto udp = DriverFactory::create(DriverFactory::Udp);
        QVERIFY(!udp->isOpen());

        auto ser = DriverFactory::create(DriverFactory::Serial);
        QVERIFY(!ser->isOpen());
    }
};

QTEST_MAIN(TestDriverFactory)
#include "test_driver_factory.moc"

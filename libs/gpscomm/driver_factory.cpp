#include "driver_factory.h"
#include "serial_driver.h"
#include "tcp_driver.h"
#include "udp_driver.h"

std::unique_ptr<IGPSDeviceDriver> DriverFactory::create(DriverType type)
{
    switch (type) {
    case Tcp:
        return std::make_unique<GpsTcpDriver>();
    case Udp:
        return std::make_unique<GpsUdpDriver>();
    case Serial:
        return std::make_unique<GpsSerialDriver>();
    }
    return nullptr;
}

std::unique_ptr<IGPSDeviceDriver> DriverFactory::create(const QString &typeName)
{
    QString lower = typeName.trimmed().toLower();
    if (lower == "tcp")
        return std::make_unique<GpsTcpDriver>();
    if (lower == "udp")
        return std::make_unique<GpsUdpDriver>();
    if (lower == "serial")
        return std::make_unique<GpsSerialDriver>();
    return nullptr;
}

QStringList DriverFactory::availableTypes()
{
    return {QStringLiteral("TCP"), QStringLiteral("UDP"), QStringLiteral("Serial")};
}

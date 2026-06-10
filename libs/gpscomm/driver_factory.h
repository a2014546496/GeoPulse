#pragma once

#include "gpscomm_global.h"
#include "igps_driver.h"
#include <memory>

/**
 * @brief 驱动工厂
 *
 * 使用示例:
 *   auto driver = DriverFactory::create(DriverFactory::Tcp);
 *   driver->open(config);
 */
class GPSCOMM_EXPORT DriverFactory
{
public:
    enum DriverType {
        Tcp    = 0,
        Udp    = 1,
        Serial = 2
    };

    /// 创建指定类型的驱动实例
    static std::unique_ptr<IGPSDeviceDriver> create(DriverType type);

    /// 从字符串创建 (支持 "tcp", "udp", "serial", 大小写不敏感)
    static std::unique_ptr<IGPSDeviceDriver> create(const QString &typeName);

    /// 返回所有可用的驱动类型名
    static QStringList availableTypes();
};

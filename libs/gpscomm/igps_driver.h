#pragma once

#include "gpscomm_global.h"
#include <QObject>
#include <QByteArray>
#include <QString>
#include <memory>

// ──────────────────────────────────────
//  驱动配置
// ──────────────────────────────────────
struct GPSCOMM_EXPORT GpsDriverConfig
{
    QString name;                  // 设备名称
    QString address;               // host:port 或串口路径
    int     baudRate    = 9600;    // 串口波特率
    int     timeoutMs   = 3000;    // 连接超时
    int     reconnectIntervalMs = 5000; // 自动重连间隔
    bool    autoReconnect = true;
};

// ──────────────────────────────────────
//  GPS 设备驱动抽象接口
// ──────────────────────────────────────
class GPSCOMM_EXPORT IGPSDeviceDriver : public QObject
{
    Q_OBJECT

public:
    explicit IGPSDeviceDriver(QObject *parent = nullptr)
        : QObject(parent) {}
    ~IGPSDeviceDriver() override = default;

    /// 打开设备连接
    virtual bool open(const GpsDriverConfig &cfg) = 0;

    /// 关闭连接, 清理资源
    virtual void close() = 0;

    /// 是否已连接
    virtual bool isOpen() const = 0;

    /// 向设备发送指令 (如配置更新频率)
    virtual bool sendCommand(const QByteArray &data) = 0;

    /// 最后一次错误信息
    virtual QString errorString() const = 0;

    /// 驱动类型标识 (如 "TCP", "UDP", "Serial")
    virtual QString driverType() const = 0;

    /// 返回当前配置
    GpsDriverConfig config() const { return m_config; }

signals:
    /// 原始字节数据到达
    void rawDataReady(const QByteArray &data);

    /// 连接状态变更
    void connectionStateChanged(bool connected);

    /// 错误通知
    void errorOccurred(const QString &error);

protected:
    GpsDriverConfig m_config;
};

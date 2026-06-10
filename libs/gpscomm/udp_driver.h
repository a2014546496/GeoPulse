#pragma once

#include "igps_driver.h"
#include <QUdpSocket>

/**
 * @brief UDP 监听模式 GPS 驱动
 *
 * 适用于: 无人机 / 网络 RTK 等通过 UDP 广播定位数据的设备
 * 绑定到指定端口监听
 */
class GPSCOMM_EXPORT GpsUdpDriver : public IGPSDeviceDriver
{
    Q_OBJECT

public:
    explicit GpsUdpDriver(QObject *parent = nullptr);
    ~GpsUdpDriver() override;

    bool open(const GpsDriverConfig &cfg) override;
    void close() override;
    bool isOpen() const override;
    bool sendCommand(const QByteArray &data) override;
    QString errorString() const override;
    QString driverType() const override { return QStringLiteral("UDP"); }

private slots:
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);

private:
    QUdpSocket *m_socket = nullptr;
    QString     m_errorString;
};

#pragma once

#include "igps_driver.h"
#include <QTcpSocket>
#include <QTimer>

/**
 * @brief TCP 客户端模式 GPS 驱动
 *
 * 适用于: 4G DTU / 网络透传模块
 * 以 TCP 客户端连接设备的 TCP Server 端口
 */
class GPSCOMM_EXPORT GpsTcpDriver : public IGPSDeviceDriver
{
    Q_OBJECT

public:
    explicit GpsTcpDriver(QObject *parent = nullptr);
    ~GpsTcpDriver() override;

    bool open(const GpsDriverConfig &cfg) override;
    void close() override;
    bool isOpen() const override;
    bool sendCommand(const QByteArray &data) override;
    QString errorString() const override;
    QString driverType() const override { return QStringLiteral("TCP"); }

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void onReconnectTimer();

private:
    void doConnect();

    QTcpSocket           *m_socket   = nullptr;
    QTimer               *m_reconnectTimer = nullptr;
    QString               m_errorString;
    bool                  m_intentionalClose = false;
    bool                  m_connecting       = false;
};

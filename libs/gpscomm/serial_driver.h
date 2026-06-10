#pragma once

#include "igps_driver.h"
#include <QSerialPort>

/**
 * @brief 串口 GPS 驱动
 *
 * 适用于: 直连 GPS 模块 (如 u-blox NEO-M8N)
 * 典型配置: 9600-8N1, 1Hz / 10Hz 更新频率
 */
class GPSCOMM_EXPORT GpsSerialDriver : public IGPSDeviceDriver
{
    Q_OBJECT

public:
    explicit GpsSerialDriver(QObject *parent = nullptr);
    ~GpsSerialDriver() override;

    bool open(const GpsDriverConfig &cfg) override;
    void close() override;
    bool isOpen() const override;
    bool sendCommand(const QByteArray &data) override;
    QString errorString() const override;
    QString driverType() const override { return QStringLiteral("Serial"); }

private slots:
    void onReadyRead();
    void onError(QSerialPort::SerialPortError error);

private:
    QSerialPort *m_port = nullptr;
    QString      m_errorString;
};

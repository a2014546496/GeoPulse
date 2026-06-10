#include "serial_driver.h"
#include <QSerialPortInfo>

GpsSerialDriver::GpsSerialDriver(QObject *parent)
    : IGPSDeviceDriver(parent)
    , m_port(new QSerialPort(this))
{
    QObject::connect(m_port, &QSerialPort::readyRead,
                     this, &GpsSerialDriver::onReadyRead);
    QObject::connect(m_port, &QSerialPort::errorOccurred,
                     this, &GpsSerialDriver::onError);
}

GpsSerialDriver::~GpsSerialDriver()
{
    close();
}

bool GpsSerialDriver::open(const GpsDriverConfig &cfg)
{
    m_config = cfg;

    m_port->setPortName(cfg.address);
    m_port->setBaudRate(cfg.baudRate);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_port->open(QIODevice::ReadWrite)) {
        m_errorString = m_port->errorString();
        emit errorOccurred(m_errorString);
        return false;
    }

    emit connectionStateChanged(true);
    return true;
}

void GpsSerialDriver::close()
{
    if (m_port->isOpen())
        m_port->close();
    emit connectionStateChanged(false);
}

bool GpsSerialDriver::isOpen() const
{
    return m_port->isOpen();
}

bool GpsSerialDriver::sendCommand(const QByteArray &data)
{
    if (!isOpen())
        return false;
    qint64 written = m_port->write(data);
    return written == data.size();
}

QString GpsSerialDriver::errorString() const
{
    return m_errorString;
}

void GpsSerialDriver::onReadyRead()
{
    QByteArray data = m_port->readAll();
    if (!data.isEmpty())
        emit rawDataReady(data);
}

void GpsSerialDriver::onError(QSerialPort::SerialPortError error)
{
    Q_UNUSED(error)
    m_errorString = m_port->errorString();
    emit errorOccurred(m_errorString);
}

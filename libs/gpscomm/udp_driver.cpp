#include "udp_driver.h"

GpsUdpDriver::GpsUdpDriver(QObject *parent)
    : IGPSDeviceDriver(parent)
    , m_socket(new QUdpSocket(this))
{
    QObject::connect(m_socket, &QUdpSocket::readyRead,
                     this, &GpsUdpDriver::onReadyRead);
    QObject::connect(m_socket, &QUdpSocket::errorOccurred,
                     this, &GpsUdpDriver::onError);
}

GpsUdpDriver::~GpsUdpDriver()
{
    close();
}

bool GpsUdpDriver::open(const GpsDriverConfig &cfg)
{
    m_config = cfg;

    quint16 port = static_cast<quint16>(cfg.address.toUShort());
    if (port == 0)
        port = 5000;

    if (!m_socket->bind(QHostAddress::AnyIPv4, port, QUdpSocket::ShareAddress)) {
        m_errorString = m_socket->errorString();
        emit errorOccurred(m_errorString);
        return false;
    }

    emit connectionStateChanged(true);
    return true;
}

void GpsUdpDriver::close()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState)
        m_socket->close();
    emit connectionStateChanged(false);
}

bool GpsUdpDriver::isOpen() const
{
    return m_socket->state() == QAbstractSocket::BoundState;
}

bool GpsUdpDriver::sendCommand(const QByteArray &data)
{
    if (!isOpen())
        return false;
    // UDP 模式下向最后收到的发送者回复
    // 简化: 广播到 255.255.255.255
    qint64 written = m_socket->writeDatagram(data, QHostAddress::Broadcast,
                                              m_config.address.toUShort());
    return written == data.size();
}

QString GpsUdpDriver::errorString() const
{
    return m_errorString;
}

void GpsUdpDriver::onReadyRead()
{
    while (m_socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(static_cast<int>(m_socket->pendingDatagramSize()));
        m_socket->readDatagram(datagram.data(), datagram.size());
        emit rawDataReady(datagram);
    }
}

void GpsUdpDriver::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    m_errorString = m_socket->errorString();
    emit errorOccurred(m_errorString);
}

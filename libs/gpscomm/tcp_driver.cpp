#include "tcp_driver.h"
#include <QHostAddress>

GpsTcpDriver::GpsTcpDriver(QObject *parent)
    : IGPSDeviceDriver(parent)
    , m_socket(new QTcpSocket(this))
    , m_reconnectTimer(new QTimer(this))
{
    m_reconnectTimer->setSingleShot(true);

    QObject::connect(m_socket, &QTcpSocket::connected,
                     this, &GpsTcpDriver::onConnected);
    QObject::connect(m_socket, &QTcpSocket::disconnected,
                     this, &GpsTcpDriver::onDisconnected);
    QObject::connect(m_socket, &QTcpSocket::readyRead,
                     this, &GpsTcpDriver::onReadyRead);
    QObject::connect(m_socket, &QTcpSocket::errorOccurred,
                     this, &GpsTcpDriver::onError);
    QObject::connect(m_reconnectTimer, &QTimer::timeout,
                     this, &GpsTcpDriver::onReconnectTimer);
}

GpsTcpDriver::~GpsTcpDriver()
{
    m_intentionalClose = true;
    close();
}

bool GpsTcpDriver::open(const GpsDriverConfig &cfg)
{
    m_config = cfg;
    m_intentionalClose = false;
    doConnect();
    return true;
}

void GpsTcpDriver::doConnect()
{
    if (m_connecting)
        return;

    QStringList parts = m_config.address.split(':');
    QString host = parts.value(0, "127.0.0.1");
    quint16 port = static_cast<quint16>(parts.value(1, "5000").toUShort());

    m_connecting = true;
    m_socket->connectToHost(host, port);
}

void GpsTcpDriver::close()
{
    m_intentionalClose = true;
    m_reconnectTimer->stop();
    m_connecting = false;

    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
        if (m_socket->state() != QAbstractSocket::UnconnectedState)
            m_socket->waitForDisconnected(2000);
    }
}

bool GpsTcpDriver::isOpen() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

bool GpsTcpDriver::sendCommand(const QByteArray &data)
{
    if (!isOpen())
        return false;
    qint64 written = m_socket->write(data);
    return written == data.size();
}

QString GpsTcpDriver::errorString() const
{
    return m_errorString;
}

// ─── Slots ──────────────────────────────────────────────

void GpsTcpDriver::onConnected()
{
    m_connecting = false;
    emit connectionStateChanged(true);
}

void GpsTcpDriver::onDisconnected()
{
    m_connecting = false;
    emit connectionStateChanged(false);

    if (!m_intentionalClose && m_config.autoReconnect)
        m_reconnectTimer->start(m_config.reconnectIntervalMs);
}

void GpsTcpDriver::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    if (!data.isEmpty())
        emit rawDataReady(data);
}

void GpsTcpDriver::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    m_connecting = false;
    m_errorString = m_socket->errorString();
    emit errorOccurred(m_errorString);

    if (!m_intentionalClose && m_config.autoReconnect)
        m_reconnectTimer->start(m_config.reconnectIntervalMs);
}

void GpsTcpDriver::onReconnectTimer()
{
    if (!m_intentionalClose && !isOpen())
        doConnect();
}

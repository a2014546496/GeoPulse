#include "logger.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QThread>

Logger *Logger::instance()
{
    static Logger s_instance;
    return &s_instance;
}

Logger::Logger(QObject *parent)
    : QObject(parent)
{
    // 默认输出到 stderr
    m_stream.setDevice(&m_file);
}

Logger::~Logger()
{
    QMutexLocker lock(&m_mutex);
    if (m_file.isOpen()) {
        m_stream.flush();
        m_file.close();
    }
}

void Logger::setLogFile(const QString &filePath, qint64 maxSizeBytes)
{
    QMutexLocker lock(&m_mutex);

    if (m_file.isOpen())
        m_file.close();

    m_filePath = filePath;
    m_maxSize  = maxSizeBytes;

    QFileInfo fi(filePath);
    QDir().mkpath(fi.absolutePath());

    m_file.setFileName(filePath);
    m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    m_stream.setDevice(&m_file);
}

void Logger::setMinLevel(Level level)
{
    QMutexLocker lock(&m_mutex);
    m_minLevel = level;
}

Logger::Level Logger::minLevel() const
{
    return m_minLevel;
}

void Logger::log(Level level, const QString &file, int line, const QString &message)
{
    if (level < m_minLevel)
        return;

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString fullMsg   = QString("[%1] [%2] %3:%4 - %5")
                            .arg(timestamp,
                                 levelString(level),
                                 file,
                                 QString::number(line),
                                 message);

    {
        QMutexLocker lock(&m_mutex);
        rotateIfNeeded();

        if (m_file.isOpen()) {
            m_stream << fullMsg << "\n";
            m_stream.flush();
        } else {
            // fallback: 无文件时输出到 stderr
            fprintf(stderr, "%s\n", qPrintable(fullMsg));
            fflush(stderr);
        }
    }

    emit logEntryAdded(static_cast<int>(level), timestamp, message);
}

void Logger::debug(const QString &msg)   { log(Debug,   {}, 0, msg); }
void Logger::info(const QString &msg)    { log(Info,    {}, 0, msg); }
void Logger::warning(const QString &msg) { log(Warning, {}, 0, msg); }
void Logger::error(const QString &msg)   { log(Error,   {}, 0, msg); }

void Logger::rotateIfNeeded()
{
    if (!m_file.isOpen() || m_maxSize <= 0)
        return;

    if (m_file.size() < m_maxSize)
        return;

    m_file.close();

    // 归档: geopause.log → geopause.1.log
    QString backup = m_filePath + ".1";
    QFile::remove(backup);
    QFile::rename(m_filePath, backup);

    m_file.setFileName(m_filePath);
    m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    m_stream.setDevice(&m_file);
}

QString Logger::levelString(Level level) const
{
    switch (level) {
    case Debug:   return QStringLiteral("DEBUG");
    case Info:    return QStringLiteral("INFO");
    case Warning: return QStringLiteral("WARN");
    case Error:   return QStringLiteral("ERROR");
    }
    return QStringLiteral("UNKNOWN");
}

#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QFile>
#include <QMutex>
#include <QTextStream>

/**
 * @brief 分级日志系统 (线程安全单例)
 *
 * 特性:
 *  - 四级日志: Debug / Info / Warning / Error
 *  - 信号输出, UI 可实时订阅
 *  - 文件滚动: 超过指定大小自动归档
 *  - 线程安全写入
 */
class Logger : public QObject
{
    Q_OBJECT

public:
    enum Level {
        Debug   = 0,
        Info    = 1,
        Warning = 2,
        Error   = 3
    };
    Q_ENUM(Level)

    static Logger *instance();

    void setLogFile(const QString &filePath, qint64 maxSizeBytes = 10 * 1024 * 1024);
    void setMinLevel(Level level);
    Level minLevel() const;

    /// Convenience for QML: set log level by int (0=Debug..3=Error)
    Q_INVOKABLE void setLogLevel(int level) { setMinLevel(static_cast<Level>(level)); }

    void log(Level level, const QString &file, int line, const QString &message);

    // 便捷宏用 — 实际项目中应配合宏 Q_FUNC_INFO 等
    void debug(const QString &msg);
    void info(const QString &msg);
    void warning(const QString &msg);
    void error(const QString &msg);

signals:
    /// 日志条目信号, UI 可 connect 订阅
    void logEntryAdded(int level, const QString &timestamp, const QString &message);

private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger() override;
    Q_DISABLE_COPY(Logger)

    void rotateIfNeeded();
    QString levelString(Level level) const;

    QMutex     m_mutex;
    QFile      m_file;
    QTextStream m_stream;
    Level      m_minLevel = Debug;
    qint64     m_maxSize  = 10 * 1024 * 1024;
    QString    m_filePath;
};

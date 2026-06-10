#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <QReadWriteLock>

/**
 * @brief JSON 配置管理器 (线程安全单例)
 *
 * 支持:
 *  - JSON 文件读写
 *  - 点号分隔的嵌套 key 访问 (如 "serial.baudrate")
 *  - 默认值回退
 *  - 线程安全读写
 */
class ConfigManager : public QObject
{
    Q_OBJECT

public:
    static ConfigManager *instance();

    bool load(const QString &filePath);
    bool save(const QString &filePath = QString());

    /// 读取配置值, 不存在时返回 defaultValue
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    /// 设置配置值
    void setValue(const QString &key, const QVariant &value);

    /// 获取全部配置 (只读)
    QVariantMap allValues() const;

    QString currentFile() const;

signals:
    void configChanged(const QString &key, const QVariant &value);
    void configLoaded(const QString &filePath);
    void configSaved(const QString &filePath);

private:
    explicit ConfigManager(QObject *parent = nullptr);
    ~ConfigManager() override = default;
    Q_DISABLE_COPY(ConfigManager)

    QVariant   getNested(const QStringList &keys, const QVariantMap &map) const;
    void       setNested(const QStringList &keys, QVariantMap &map, const QVariant &value);
    void       mergeMap(QVariantMap &target, const QVariantMap &source);

    mutable QReadWriteLock m_lock;
    QVariantMap  m_data;
    QString      m_currentFile;
};

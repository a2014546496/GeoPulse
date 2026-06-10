#include "config_manager.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>

ConfigManager *ConfigManager::instance()
{
    static ConfigManager s_instance;
    return &s_instance;
}

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
{
}

bool ConfigManager::load(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit configLoaded({});
        return false;
    }

    QByteArray raw = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
    if (err.error != QJsonParseError::NoError) {
        emit configLoaded({});
        return false;
    }

    {
        QWriteLocker locker(&m_lock);
        m_data = doc.object().toVariantMap();
        m_currentFile = filePath;
    }

    emit configLoaded(filePath);
    return true;
}

bool ConfigManager::save(const QString &filePath)
{
    QString path = filePath.isEmpty() ? m_currentFile : filePath;
    if (path.isEmpty())
        return false;

    QJsonDocument doc;
    {
        QReadLocker locker(&m_lock);
        doc = QJsonDocument(QJsonObject::fromVariantMap(m_data));
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    emit configSaved(path);
    return true;
}

QVariant ConfigManager::value(const QString &key, const QVariant &defaultValue) const
{
    QReadLocker locker(&m_lock);
    QStringList parts = key.split('.');
    QVariant result = getNested(parts, m_data);
    return result.isValid() ? result : defaultValue;
}

void ConfigManager::setValue(const QString &key, const QVariant &value)
{
    {
        QWriteLocker locker(&m_lock);
        QStringList parts = key.split('.');
        setNested(parts, m_data, value);
    }
    emit configChanged(key, value);
}

QVariantMap ConfigManager::allValues() const
{
    QReadLocker locker(&m_lock);
    return m_data;
}

QString ConfigManager::currentFile() const
{
    QReadLocker locker(&m_lock);
    return m_currentFile;
}

QVariant ConfigManager::getNested(const QStringList &keys, const QVariantMap &map) const
{
    if (keys.isEmpty())
        return QVariant();

    QVariant val = map.value(keys.first());
    if (keys.size() == 1)
        return val;

    QVariantMap inner = val.toMap();
    return getNested(keys.mid(1), inner);
}

void ConfigManager::setNested(const QStringList &keys, QVariantMap &map, const QVariant &value)
{
    if (keys.isEmpty())
        return;

    if (keys.size() == 1) {
        map[keys.first()] = value;
        return;
    }

    QVariantMap inner = map.value(keys.first()).toMap();
    setNested(keys.mid(1), inner, value);
    map[keys.first()] = inner;
}

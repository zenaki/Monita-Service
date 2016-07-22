#ifndef CONFIG_H
#define CONFIG_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QList>
#include <QDebug>

class configData;

class config
{
public:
    config();

    enum SaveFormat {
        Json, Binary
    };

    QStringList read(QString obj);
    void write(QJsonObject &json) const;    //Default
    bool loadConfig(config::SaveFormat saveFormat);
    bool saveConfig(config::SaveFormat saveFormat) const;

    void createFile(QString path);

    void write_source_default(QJsonObject &json) const;
    void write_redis_default(QJsonObject &json) const;
private:
    QList<config> c_sourceList;
    QList<config> c_redisList;

    QJsonDocument JsonDoc;
    QByteArray data_json;
};

#endif // CONFIG_H

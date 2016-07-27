#ifndef CONFIG_H
#define CONFIG_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QDebug>

#include "util/utama.h"

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
private:
    QList<config> c_sourceList;
    QList<config> c_redisList;

    QJsonDocument JsonDoc;
    QByteArray data_json;
};

#endif // CONFIG_H

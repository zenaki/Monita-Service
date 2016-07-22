#include "config.h"

config::config()
{
}

QStringList config::read(QString obj)
{
    QStringList result;
    if (loadConfig(config::Json)) {
        JsonDoc = QJsonDocument::fromJson(data_json);
        QJsonObject object = JsonDoc.object();
        QJsonValue value = object.value(obj);
        if (obj == "redis") {
            result.append(value.toObject().value("IP").toString());
            result.append(QString::number(value.toObject().value("PORT").toInt()));
        } else if (obj == "source") {
            if (object.value(obj).isArray()) {
                QJsonArray array = value.toArray();
                foreach (const QJsonValue & v, array) {
                    result.append(QString::number(v.toObject().value("ID").toInt()));
                    result.append(v.toObject().value("IP").toString());
                    result.append(QString::number(v.toObject().value("PORT").toInt()));
                    result.append(QString::number(v.toObject().value("SLAVE_ID").toInt()));
                    result.append(QString::number(v.toObject().value("FUNCT_ID").toInt()));
                    result.append(QString::number(v.toObject().value("START").toInt()));
                    result.append(QString::number(v.toObject().value("COILS").toInt()));
                }
            } else {
                result.append(QString::number(value.toObject().value("ID").toInt()));
                result.append(value.toObject().value("IP").toString());
                result.append(QString::number(value.toObject().value("PORT").toInt()));
                result.append(QString::number(value.toObject().value("SLAVE_ID").toInt()));
                result.append(QString::number(value.toObject().value("FUNCT_ID").toInt()));
                result.append(QString::number(value.toObject().value("START").toInt()));
                result.append(QString::number(value.toObject().value("COILS").toInt()));
            }
        }
    }
//    mPlayer.read(json["player"].toObject());

//    mLevels.clear();
//    QJsonArray levelArray = json["levels"].toArray();
//    for (int levelIndex = 0; levelIndex < levelArray.size(); ++levelIndex) {
//        QJsonObject levelObject = levelArray[levelIndex].toObject();
//        Level level;
//        level.read(levelObject);
//        mLevels.append(level);
//    }
    return result;
}

void config::write(QJsonObject &json) const //Default
{
    QJsonArray sourceArray;
    foreach (const config source, c_sourceList) {
        QJsonObject sourceObject;
        source.write_source_default(sourceObject);
        sourceArray.append(sourceObject);
    }
    json["source"] = sourceArray;

    QJsonArray redisArray;
    foreach (const config redis, c_redisList) {
        QJsonObject redisObject;
        redis.write_redis_default(redisObject);
        redisArray.append(redisObject);
    }
    json["redis"] = redisArray;
}

bool config::loadConfig(config::SaveFormat saveFormat)
{
    QFile loadFile(saveFormat == Json
        ? QStringLiteral("monita-cfg.json")
        : QStringLiteral("monita-cfg.dat"));

    if (loadFile.open(QIODevice::ReadWrite)) {
        QByteArray readFile = loadFile.readAll();
        if (readFile.isEmpty()) {
            QJsonObject configObject;
            write(configObject);
            QJsonDocument saveDoc(configObject);
            loadFile.write(saveFormat == Json
                ? saveDoc.toJson()
                : saveDoc.toBinaryData());
        }
        data_json = readFile;
    }

//    read(loadDoc.object());

    return true;
}
//! [3]

//! [4]
bool config::saveConfig(config::SaveFormat saveFormat) const
{
    QFile saveFile(saveFormat == Json
        ? QStringLiteral("monita-cfg.json")
        : QStringLiteral("monita-cfg.dat"));

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Couldn't save config ..";
        return false;
    }

    QJsonObject configObject;
    write(configObject);
    QJsonDocument saveDoc(configObject);
    saveFile.write(saveFormat == Json
        ? saveDoc.toJson()
        : saveDoc.toBinaryData());

    return true;
}

void config::write_source_default(QJsonObject &json) const
{
    QString ip = "192.168.3.241";
    json["IP"] = ip;
    json["PORT"] = 502;
    json["SLAVE_ID"] = 2;
    json["FUNCT_ID"] = 3;
    json["START"] = 3027;
    json["COILS"] = 6;
}

void config::write_redis_default(QJsonObject &json) const
{
    QString ip = "127.0.0.1";
    json["IP"] = ip;
    json["PORT"] = 6379;
}

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
            if (object.value(obj).isArray()) {
                QJsonArray array = value.toArray();
                foreach (const QJsonValue & v, array) {
                    result.append(v.toObject().value("IP").toString());
                    result.append(QString::number(v.toObject().value("PORT").toInt()));
                }
            } else {
                result.append(value.toObject().value("IP").toString());
                result.append(QString::number(value.toObject().value("PORT").toInt()));
            }
        } else if (obj == "source") {
            if (object.value(obj).isArray()) {
                QJsonArray array = value.toArray();
                foreach (const QJsonValue & v, array) {
                    result.append(v.toObject().value("IP").toString());
                    result.append(QString::number(v.toObject().value("PORT").toInt()));
                    result.append(QString::number(v.toObject().value("SLAVE_ID").toInt()));
                    result.append(QString::number(v.toObject().value("FUNCT_ID").toInt()));
                    result.append(QString::number(v.toObject().value("START").toInt()));
                    result.append(QString::number(v.toObject().value("COILS").toInt()));
                    result.append(v.toObject().value("MODE").toString());
                }
            } else {
                result.append(value.toObject().value("IP").toString());
                result.append(QString::number(value.toObject().value("PORT").toInt()));
                result.append(QString::number(value.toObject().value("SLAVE_ID").toInt()));
                result.append(QString::number(value.toObject().value("FUNCT_ID").toInt()));
                result.append(QString::number(value.toObject().value("START").toInt()));
                result.append(QString::number(value.toObject().value("COILS").toInt()));
                result.append(value.toObject().value("MODE").toString());
            }
        } else if (obj == "config") {
            if (object.value(obj).isArray()) {
                QJsonArray array = value.toArray();
                foreach (const QJsonValue & v, array) {
                    result.append(QString::number(v.toObject().value("DB_PERIOD").toInt()));
                    result.append(QString::number(v.toObject().value("INTERVAL").toInt()));
                    result.append(QString::number(v.toObject().value("TIMESTAMP").toInt()));
                    result.append(v.toObject().value("LOG_PATH").toString());
                }
            } else {
                result.append(QString::number(value.toObject().value("DB_PERIOD").toInt()));
                result.append(QString::number(value.toObject().value("INTERVAL").toInt()));
                result.append(QString::number(value.toObject().value("TIMESTAMP").toInt()));
                result.append(value.toObject().value("LOG_PATH").toString());
            }
        }
    }
    return result;
}

void config::write(QJsonObject &json) const //Default
{
    QJsonArray sourceArray;
    QJsonObject sourceObject;
    QString temp = "192.168.1.1";
    sourceObject["IP"] = temp;
    sourceObject["PORT"] = 502;
    sourceObject["SLAVE_ID"] = 1;
    sourceObject["FUNCT_ID"] = 3;
    sourceObject["START"] = 1;
    sourceObject["COILS"] = 1;
    temp = "TCP";
    sourceObject["MODE"] = temp;
    sourceArray.append(sourceObject);
    json["source"] = sourceArray;

    QJsonArray redisArray;
    QJsonObject redisObject;
    temp = "127.0.0.1";
    redisObject["IP"] = temp;
    redisObject["PORT"] = 6379;
    redisArray.append(redisObject);
    json["redis"] = redisArray;

    QJsonArray configArray;
    QJsonObject configObject;
    configObject["INTERVAL"] = 1000;    //milis
    configObject["DB_PERIOD"] = 60;     //detik
    configObject["TIMESTAMP"] = 3;     //TimeStamp
    temp = "monita_configuration/log.txt";
    configObject["LOG_PATH"] = temp;
    configArray.append(configObject);
    json["config"] = configArray;
}

bool config::loadConfig(config::SaveFormat saveFormat)
{
    QString path;
    if (saveFormat == Json) {
        path = PATH_CONFIGURATION_JSON;
    } else {
        path = PATH_CONFIGURATION_BIN;
    }
    QFile loadFile(path);
    if (!loadFile.exists()) {
        QDir dir;
        dir.mkpath("monita_configuration");
    }
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
        return true;
    } else {
        return false;
    }

//    read(loadDoc.object());
}
//! [3]

//! [4]
bool config::saveConfig(config::SaveFormat saveFormat) const
{
    QString path;
    if (saveFormat == Json) {
        path = PATH_CONFIGURATION_JSON;
    } else {
        path = PATH_CONFIGURATION_BIN;
    }
    QFile saveFile(path);
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

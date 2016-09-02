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
        if (obj == "REDIS") {
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
        } else if (obj == "SOURCE") {
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
                    result.append(QString::number(v.toObject().value("BYTE").toInt()));
                    result.append(v.toObject().value("TYPE").toString());
                }
            } else {
                result.append(value.toObject().value("IP").toString());
                result.append(QString::number(value.toObject().value("PORT").toInt()));
                result.append(QString::number(value.toObject().value("SLAVE_ID").toInt()));
                result.append(QString::number(value.toObject().value("FUNCT_ID").toInt()));
                result.append(QString::number(value.toObject().value("START").toInt()));
                result.append(QString::number(value.toObject().value("COILS").toInt()));
                result.append(value.toObject().value("MODE").toString());
                result.append(QString::number(value.toObject().value("BYTE").toInt()));
                result.append(value.toObject().value("TYPE").toString());
            }
        } else if (obj == "CONFIG") {
            if (object.value(obj).isArray()) {
                QJsonArray array = value.toArray();
                foreach (const QJsonValue & v, array) {
                    result.append(QString::number(v.toObject().value("DB_PERIOD").toInt()));
                    result.append(QString::number(v.toObject().value("INTERVAL").toInt()));
                    result.append(QString::number(v.toObject().value("TIMESTAMP").toInt()));
                    result.append(v.toObject().value("REDIS_KEY").toString());
                    result.append(v.toObject().value("TABLE_NAME").toString());
                    result.append(QString::number(v.toObject().value("WEBSOCKET_PORT").toInt()));
                }
            } else {
                result.append(QString::number(value.toObject().value("DB_PERIOD").toInt()));
                result.append(QString::number(value.toObject().value("INTERVAL").toInt()));
                result.append(QString::number(value.toObject().value("TIMESTAMP").toInt()));
                result.append(value.toObject().value("REDIS_KEY").toString());
                result.append(value.toObject().value("TABLE_NAME").toString());
                result.append(QString::number(value.toObject().value("WEBSOCKET_PORT").toInt()));
            }
        } else if (obj == "FUNCT") {
            if (object.value(obj).isArray()) {
                QJsonArray array = value.toArray();
                foreach (const QJsonValue & v, array) {
                    result.append(v.toObject().value("LUA_FILE").toString());
                }
            } else {
                result.append(value.toObject().value("LUA_FILE").toString());
            }
        } else if (obj == "SKYWAVE") {
            if (object.value(obj).isArray()) {
                QJsonArray array = value.toArray();
                foreach (const QJsonValue & v, array) {
                    result.append(QString::number(v.toObject().value("INTERVAL").toInt()));
                    result.append(v.toObject().value("MODE").toString());
                    result.append(v.toObject().value("TYPE").toString());
                }
            } else {
                result.append(QString::number(value.toObject().value("INTERVAL").toInt()));
                result.append(value.toObject().value("MODE").toString());
                result.append(value.toObject().value("TYPE").toString());
            }
        }
    }
    return result;
}

void config::write(QJsonObject &json) const //Default
{
    QJsonArray sourceArray;
    QJsonObject sourceObject;
    sourceObject["IP"] = QString("192.168.1.1");
    sourceObject["PORT"] = 502;
    sourceObject["SLAVE_ID"] = 1;
    sourceObject["FUNCT_ID"] = 3;
    sourceObject["START"] = 1;
    sourceObject["COILS"] = 1;
    sourceObject["MODE"] = QString("TCP");
    sourceObject["BYTE"] = 2;
    sourceObject["TYPE"] = QString("FLOAT");
    sourceArray.append(sourceObject);
    json["SOURCE"] = sourceArray;

    QJsonArray redisArray;
    QJsonObject redisObject;
    redisObject["IP"] = QString("127.0.0.1");
    redisObject["PORT"] = 6379;
    redisArray.append(redisObject);
    json["REDIS"] = redisArray;

    QJsonArray configArray;
    QJsonObject configObject;
    configObject["INTERVAL"] = 1000;    //milis
    configObject["DB_PERIOD"] = 60;     //detik
    configObject["TIMESTAMP"] = 3;     //TimeStamp
    configObject["REDIS_KEY"] = QString("data_jaman_");
    configObject["TABLE_NAME"] = QString("data_harian_");
    configObject["WEBSOCKET_PORT"] = 1234;
    configArray.append(configObject);
    json["CONFIG"] = configArray;
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
        dir.mkpath(".MonSerConfig");
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

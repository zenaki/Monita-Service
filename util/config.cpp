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
        }
//        else if (obj == "SOURCE") {
//            if (object.value(obj).isArray()) {
//                QJsonArray array = value.toArray();
//                foreach (const QJsonValue & v, array) {
//                    result.append(v.toObject().value("IP").toString());
//                    result.append(QString::number(v.toObject().value("PORT").toInt()));
//                    result.append(QString::number(v.toObject().value("SLAVE_ID").toInt()));
//                    result.append(QString::number(v.toObject().value("FUNCT_ID").toInt()));
//                    result.append(QString::number(v.toObject().value("START").toInt()));
//                    result.append(QString::number(v.toObject().value("COILS").toInt()));
//                    result.append(v.toObject().value("MODE").toString());
//                    result.append(QString::number(v.toObject().value("BYTE").toInt()));
//                    result.append(v.toObject().value("TYPE").toString());
//                    result.append(v.toObject().value("SN").toString());
//                }
//            } else {
//                result.append(value.toObject().value("IP").toString());
//                result.append(QString::number(value.toObject().value("PORT").toInt()));
//                result.append(QString::number(value.toObject().value("SLAVE_ID").toInt()));
//                result.append(QString::number(value.toObject().value("FUNCT_ID").toInt()));
//                result.append(QString::number(value.toObject().value("START").toInt()));
//                result.append(QString::number(value.toObject().value("COILS").toInt()));
//                result.append(value.toObject().value("MODE").toString());
//                result.append(QString::number(value.toObject().value("BYTE").toInt()));
//                result.append(value.toObject().value("TYPE").toString());
//                result.append(value.toObject().value("SN").toString());
//            }
//        }
        else if (obj == "APP") {
            if (object.value(obj).isArray()) {
                QJsonArray array = value.toArray();
                foreach (const QJsonValue & v, array) {
                    if (v.toObject().value("SOURCE").isArray()) {
                        QJsonArray sourceArray = v.toObject().value("SOURCE").toArray();
                        foreach (const QJsonValue & vSource, sourceArray) {
                            if (vSource.toObject().value("ARGV").isArray()) {
                                QJsonArray argv = vSource.toObject().value("ARGV").toArray();
                                QString arg = "";
                                foreach (const QJsonValue & vARGV, argv) {
                                    arg = arg + vARGV.toObject().value("ARG").toString() + " ";
                                }
                                result.append(v.toObject().value("PATH").toString() + " " + arg);
                                result.append(vSource.toObject().value("SN").toString());
                            }
                        }
                    }
                }
            }
        }
        else if (obj == "CONFIG") {
            if (object.value(obj).isArray()) {
                QJsonArray array = value.toArray();
                foreach (const QJsonValue & v, array) {
                    result.append(QString::number(v.toObject().value("DB_PERIOD").toInt()));
                    result.append(QString::number(v.toObject().value("INTERVAL").toInt()));
                    result.append(QString::number(v.toObject().value("TIMESTAMP").toInt()));
                    result.append(v.toObject().value("REDIS_KEY").toString());
                    result.append(v.toObject().value("TABLE_NAME").toString());
                    result.append(QString::number(v.toObject().value("WEBSOCKET_PORT").toInt()));
                    result.append(QString::number(v.toObject().value("INTERVAL_SKYWAVE").toInt()));
                    result.append(QString::number(v.toObject().value("DEBUG_TCPMODBUS_DATA").toBool()));
                    result.append(QString::number(v.toObject().value("DEBUG_TCPMODBUS_DATABASE").toBool()));
                    result.append(QString::number(v.toObject().value("DEBUG_SKYWAVE_DATA").toBool()));
                    result.append(QString::number(v.toObject().value("DEBUG_SKYWAVE_DATABASE").toBool()));
                }
            } else {
                result.append(QString::number(value.toObject().value("DB_PERIOD").toInt()));
                result.append(QString::number(value.toObject().value("INTERVAL").toInt()));
                result.append(QString::number(value.toObject().value("TIMESTAMP").toInt()));
                result.append(value.toObject().value("REDIS_KEY").toString());
                result.append(value.toObject().value("TABLE_NAME").toString());
                result.append(QString::number(value.toObject().value("WEBSOCKET_PORT").toInt()));
                result.append(QString::number(value.toObject().value("INTERVAL_SKYWAVE").toInt()));
                result.append(QString::number(value.toObject().value("DEBUG_TCPMODBUS_DATA").toInt()));
                result.append(QString::number(value.toObject().value("DEBUG_TCPMODBUS_DATABASE").toInt()));
                result.append(QString::number(value.toObject().value("DEBUG_SKYWAVE_DATA").toInt()));
                result.append(QString::number(value.toObject().value("DEBUG_SKYWAVE_DATABASE").toInt()));
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
        }
    }
    return result;
}

void config::write(QJsonObject &json) const //Default
{
//    QJsonArray sourceArray;
//    QJsonObject sourceObject;
//    sourceObject["IP"] = QString("192.168.1.1");
//    sourceObject["PORT"] = 502;
//    sourceObject["SLAVE_ID"] = 1;
//    sourceObject["FUNCT_ID"] = 3;
//    sourceObject["START"] = 1;
//    sourceObject["COILS"] = 1;
//    sourceObject["MODE"] = QString("TCP");
//    sourceObject["BYTE"] = 2;
//    sourceObject["TYPE"] = QString("FLOAT");
//    sourceObject["SN"] = QString("XXXX-XXXXXX-XXX-XXXXX");
//    sourceArray.append(sourceObject);
//    json["SOURCE"] = sourceArray;

    QJsonArray appArray;
    QJsonObject appObject;
    QJsonArray sourceArray;
    QJsonObject sourceObject;
    QJsonArray argv;
    QJsonObject arg;
    arg["ARG"] = QString("-tcp"); argv.append(arg); arg.remove("ARG");
    arg["ARG"] = QString("--ip-address 192.168.1.1"); argv.append(arg); arg.remove("ARG");
    arg["ARG"] = QString("--port 502"); argv.append(arg); arg.remove("ARG");
    arg["ARG"] = QString("--slave-id 1"); argv.append(arg); arg.remove("ARG");
    arg["ARG"] = QString("--function-code 3"); argv.append(arg); arg.remove("ARG");
    arg["ARG"] = QString("--start-address 1000"); argv.append(arg); arg.remove("ARG");
    arg["ARG"] = QString("--num-of-coils 20"); argv.append(arg); arg.remove("ARG");
    arg["ARG"] = QString("--num-of-bytes 2"); argv.append(arg); arg.remove("ARG");
    arg["ARG"] = QString("--type-conversion FLOAT"); argv.append(arg); arg.remove("ARG");
    sourceObject["ARGV"] = argv;
    sourceObject["SN"] = QString("XXXX-XXXXXX-XXX-XXXXX");
    sourceArray.append(sourceObject);
    appObject["PATH"] = QString("plugins/ModBus");
    appObject["SOURCE"] = sourceArray;
    appArray.append(appObject);
    json["APP"] = appArray;

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
    configObject["REDIS_KEY"] = QString("data_");
    configObject["TABLE_NAME"] = QString("data_");
    configObject["WEBSOCKET_PORT"] = 1234;
    configObject["INTERVAL_SKYWAVE"] = 2;
    configObject["DEBUG_TCPMODBUS_DATA"] = false;
    configObject["DEBUG_TCPMODBUS_DATABASE"] = false;
    configObject["DEBUG_SKYWAVE_DATA"] = false;
    configObject["DEBUG_SKYWAVE_DATABASE"] = false;
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

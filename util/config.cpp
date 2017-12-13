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
//                    result.append(QString::number(v.toObject().value("INTERVAL_SKYWAVE").toInt()));
                    result.append(QString::number(v.toObject().value("DEBUG_DATA").toBool()));
                    result.append(QString::number(v.toObject().value("DEBUG_DATABASE").toBool()));
                    result.append(QString::number(v.toObject().value("HTTP_SERVER_PORT").toInt()));
                }
            } else {
                result.append(QString::number(value.toObject().value("DB_PERIOD").toInt()));
                result.append(QString::number(value.toObject().value("INTERVAL").toInt()));
                result.append(QString::number(value.toObject().value("TIMESTAMP").toInt()));
                result.append(value.toObject().value("REDIS_KEY").toString());
                result.append(value.toObject().value("TABLE_NAME").toString());
                result.append(QString::number(value.toObject().value("WEBSOCKET_PORT").toInt()));
//                result.append(QString::number(value.toObject().value("INTERVAL_SKYWAVE").toInt()));
                result.append(QString::number(value.toObject().value("DEBUG_DATA").toInt()));
                result.append(QString::number(value.toObject().value("DEBUG_DATABASE").toInt()));
                result.append(QString::number(value.toObject().value("HTTP_SERVER_PORT").toInt()));
            }
        } else if (obj == "RPT_GEN") {
          result.append(object.value(obj).toString());
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
    configObject["HTTP_SERVER_PORT"] = 1309;
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

plugins config::get_plugins() {
    struct plugins plg;

    for (int i = 0; i < MAX_PLUGINS; i++) {
        plg.path[i].clear();
        plg.database[i] = false;
        plg.time_periode[i] = 0;
        plg.arg[i].clear();
    }

    if (loadConfig(config::Json)) {
        JsonDoc = QJsonDocument::fromJson(data_json);
        QJsonObject object = JsonDoc.object();
        QJsonValue value = object.value("APP");
        if (object.value("APP").isArray()) {
            QJsonArray array = value.toArray(); int index = 0;
            foreach (const QJsonValue & v, array) {
                if (v.toObject().value("DATABASE_SOURCE").toBool()) {
                    plg.path[index] = v.toObject().value("PATH").toString();
                    plg.database[index] = v.toObject().value("DATABASE_SOURCE").toBool();
                    plg.time_periode[index] = v.toObject().value("TIME_PERIODE").toInt();
//                    plg.arg[index] = this->get_plugins_from_database();
                    plg = this->get_plugins_from_database(plg, index);
                    for (int i = 0; i < plg.arg[index].length(); i++) {
                        plg.arg[index].replace(i, plg.arg[index].at(i) + "-t " + QString::number(plg.time_periode[index]));
                    }
                } else {
                    plg.path[index] = v.toObject().value("PATH").toString();
                    plg.database[index] = v.toObject().value("DATABASE_SOURCE").toBool();
                    plg.time_periode[index] = v.toObject().value("TIME_PERIODE").toInt();
                    if (v.toObject().value("SOURCE").isArray()) {
                        QJsonArray sourceArray = v.toObject().value("SOURCE").toArray();
                        foreach (const QJsonValue & vSource, sourceArray) {
                            if (vSource.toObject().value("ARGV").isArray()) {
                                QJsonArray argv = vSource.toObject().value("ARGV").toArray();
                                QString arg = "";
                                foreach (const QJsonValue & vARGV, argv) {
                                    arg = arg + vARGV.toObject().value("ARG").toString() + " ";
                                }
                                plg.arg[index].append(arg);
//                                result.append(v.toObject().value("PATH").toString() + " " + arg);
//                                result.append(vSource.toObject().value("SN").toString());
                            }
                            plg.sn[index].append(vSource.toObject().value("SN").toString());
                        }
                    }
                }
                index++;
            }
        }
    }
    return plg;
}

plugins config::get_plugins_from_database(struct plugins plg, int index) {
    QSqlDatabase db;
    db = mysql.connect_db("Config");
    db.open();
    QSqlQuery q(QSqlDatabase::database(db.connectionName()));
//    int last_id = 0;

    QDateTime dt1 = QDateTime::currentDateTime();
    QDateTime dt2 = dt1.toUTC();
    dt1.setTimeSpec(Qt::UTC);
    int offset = dt2.secsTo(dt1) / 3600;

    if (!q.exec("call get_skywave_parameter();")) {
        return plg;
    } else {
        while (q.next()) {
//            -g http://m2prime.aissat.com/RestMessages.svc/get_return_messages.json/ -aid 150103286 -pwd ZRM3B9SSDI -sm 128;1 -s 2017-03-27#03:43:02 -t 10000
//            if (last_id != q.value(0).toInt()) {
//                last_id = q.value(0).toInt();
//                plg.id[index].append(QString::number(last_id));
                plg.id[index].append(q.value(0).toString());
                plg.sn[index].append(q.value(1).toString());
                plg.arg[index].append(
//                            "-g " + q.value(2).toString().toLatin1() + "get_return_messages.json/ " +
                            "-g " + q.value(2).toString().toLatin1() + " " +
                            "-aid " + q.value(3).toString().toLatin1() + " " +
                            "-pwd " + q.value(4).toString().toLatin1() + " " +
                            "-sm " + q.value(5).toString().toLatin1() + " " +
                            "-s " + QDateTime::fromTime_t(q.value(6).toInt()-(offset*3600)).toString("yyyy-MM-dd#HH:mm:dd") + " "
                            );
//                qDebug() << plg.arg[index];
//                qDebug() << q.value(6).toInt();
//                qDebug() << "test";
//            } else {
//                plg.sn[index].append(q.value(1).toString());
//                plg.arg[index].append(
////                            "-g " + q.value(2).toString().toLatin1() + "get_return_messages.json/ " +
//                            "-g " + q.value(2).toString().toLatin1() + " " +
//                            "-aid " + q.value(3).toString().toLatin1() + " " +
//                            "-pwd " + q.value(4).toString().toLatin1() + " " +
//                            "-sm " + q.value(5).toString().toLatin1() + " " +
//                            "-s " + QDateTime::fromTime_t(q.value(6).toInt()-(offset*3600)).toString("yyyy-MM-dd#HH:mm:dd") + " "
//                            );
//            }
//            result.append(
//                        "-g " + q.value(0).toString().toLatin1() + "get_return_messages.json/ " +
//                        "-aid " + q.value(1).toString().toLatin1() + " " +
//                        "-pwd " + q.value(2).toString().toLatin1() + " " +
//                        "-sm " + q.value(3).toString().toLatin1() + " " +
//                        "-s " + QDateTime::fromTime_t(q.value(4).toInt()).toString("yyyy-MM-dd#HH:mm:dd") + " "
////                        q.value(5).toString().toLatin1() + ";" +
////                        q.value(6).toString().toLatin1() + ";" +
////                        q.value(7).toString().toLatin1() + ";" +
////                        q.value(8).toString().toLatin1() + ";" +
////                        q.value(9).toString().toLatin1() + ";" +
////                        q.value(10).toString().toLatin1() + ";" +
////                        q.value(11).toString().toLatin1() + ";" +
////                        q.value(12).toString().toLatin1()
//                          );
        }
    }

    db.close();

    return plg;
}

void config::set_nextutc_skywave(QString id, QString nextUTC) {
    QSqlDatabase db;
    db = mysql.connect_db("SkyWave");
    db.open();
    QSqlQuery q(QSqlDatabase::database(db.connectionName()));

    if (!q.exec("call set_skywave_nextutc("+id+","+nextUTC+");")) {
        return;
    } else {
        return;
    }

    db.close();
}

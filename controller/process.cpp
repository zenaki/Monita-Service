#include "process.h"

process::process(QObject *parent) : QObject(parent)
{

}

void process::doSetup(QThread &cThread, QStringList plugins_id, QString plugins_path, QStringList arg, QStringList sn, int time_periode)
{
    connect(&cThread,SIGNAL(started()),this,SLOT(doWork()));
    id = plugins_id;
    Plugin = plugins_path;
    Argv = arg;
    SN = sn;

    logsheet.clear();
    for (int i = 0; i < Argv.length(); i++) logsheet.append("0");

    monita_cfg.config = cfg.read("CONFIG");

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(doWork()));
    t->start(time_periode);


}

void process::doWork()
{
//    if (QDateTime::currentDateTime().toString("ss").toInt() <= 20 && QDateTime::currentDateTime().toString("ss").toInt() >= 50) {
//        logsheet.clear();
//        for (int i = 0; i < Argv.length(); i++) logsheet.append("0");
//    }
    for (int i = 0; i < Argv.length(); i++) {
        log.write("Process", Plugin + " " + Argv.at(i), monita_cfg.config.at(6).toInt());
        QProcess proc;
        proc.start("./" + Plugin + " " + Argv.at(i));
        proc.waitForFinished(); // sets current thread to sleep and waits for pingProcess end
        QString output(proc.readAllStandardOutput());
        QJsonObject obj = this->ObjectFromString(output);
//        proc.deleteLater();
        printf("%s", output.toLatin1().data());

        if (!obj.value("monita").isUndefined()) {
//            log.write("Monita", Plugin + " " + Argv.at(i), monita_cfg.config.at(6).toInt());
            this->monita_parse(obj, i);

        } else if (!obj.value("skywave").isUndefined()) {
//            log.write("SkyWave", Plugin + " " + Argv.at(i), monita_cfg.config.at(6).toInt());
            this->skywave_parse(obj, i);

        } else if (!obj.value("ERR").isUndefined()) {
//            log.write("Process", obj.value("ERR").toString(), monita_cfg.config.at(6).toInt());
        }
    }
}

QJsonObject process::ObjectFromString(QString in)
{
    QJsonObject obj;

    QJsonDocument doc = QJsonDocument::fromJson(in.toUtf8());

    // check validity of the document
    if (!doc.isNull()) {
        if (doc.isObject()) {
            obj = doc.object();
        }
    }

    return obj;
}

void process::monita_parse(QJsonObject obj, int index) {
    QStringList titik_ukur;
    QString SerialNumber = SN.at(index);

    QStringList redis_config = cfg.read("REDIS");
    QStringList request = rds.reqRedis("hlen monita_service:parsing_ref", redis_config.at(0), redis_config.at(1).toInt());
    int redis_len = request.at(0).toInt();
    request = rds.reqRedis("hgetall monita_service:parsing_ref", redis_config.at(0), redis_config.at(1).toInt(), redis_len*2);
    for (int j = 0; j < request.length(); j+=2) {
        QStringList temp = request.at(j).split(";");
        if (SerialNumber == temp.at(0)) {
            titik_ukur.insert(temp.at(1).toInt()-1, request.at(j+1));
        }
    }

    if (titik_ukur.length() > 0) {
        QStringList result[2];
        if (obj.value("monita").isArray()) {
            QJsonArray array = obj.value("monita").toArray();
            foreach (const QJsonValue & v, array) {
                result[0].append(v.toObject().value("value").toString());
                result[1].append(v.toObject().value("epochtime").toString());
            }
            if (result[0].length() > 0) {
                if (titik_ukur.length() > result[1].length()) {
                    for (int j = 0; j < result[1].length(); j++) {
                        rds.reqRedis("hset monita_service:history:" + monita_cfg.config.at(3) +
                                      titik_ukur.at(j) + " " +
                                      QString::number(QDateTime::currentMSecsSinceEpoch())
                                        .left(QString::number(QDateTime::currentMSecsSinceEpoch()).length() - 3) +
//                                      QString::number(QDateTime::currentMSecsSinceEpoch()) +
                                      " " +
                                      result[0].at(j), redis_config.at(0), redis_config.at(1).toInt());
//                        if (logsheet.at(index).toInt() <= monita_cfg.config.at(2).toInt()) {
                            rds.reqRedis("hset monita_service:temp " +
                                          titik_ukur.at(j) +
                                          "_" +
                                          QString::number(QDateTime::currentMSecsSinceEpoch())
                                            .left(QString::number(QDateTime::currentMSecsSinceEpoch()).length() - 3) +
//                                          QString::number(QDateTime::currentMSecsSinceEpoch()) +
                                          " " +
                                          result[0].at(j), redis_config.at(0), redis_config.at(1).toInt());
                            log.write("Database",
                                      QString::number(j) + " - " +
                                      titik_ukur.at(j) + " - " +
                                      result[0].at(j),
                                      monita_cfg.config.at(6).toInt());
                            logsheet.replace(index, QString::number(logsheet.at(index).toInt()+1));
//                        }
                        rds.reqRedis("hset monita_service:vismon " +
                                     SerialNumber + ";" +
                                     titik_ukur.at(j) +
                                     " " +
                                     QString::number(QDateTime::currentMSecsSinceEpoch())
                                       .left(QString::number(QDateTime::currentMSecsSinceEpoch()).length() - 3) + ";" +
//                                     QString::number(QDateTime::currentMSecsSinceEpoch()) + ";" +
                                     result[0].at(j), redis_config.at(0), redis_config.at(1).toInt());
                        log.write("TcpModbus",
                                  QString::number(j) + " - " +
                                  titik_ukur.at(j) + " - " +
                                  result[0].at(j),
                                  monita_cfg.config.at(6).toInt());
                    }
                } else {
                    for (int j = 0; j < titik_ukur.length(); j++) {
                        rds.reqRedis("hset monita_service:history:" + monita_cfg.config.at(3) +
                                      titik_ukur.at(j) + " " +
                                      QString::number(QDateTime::currentMSecsSinceEpoch())
                                        .left(QString::number(QDateTime::currentMSecsSinceEpoch()).length() - 3) +
//                                      QString::number(QDateTime::currentMSecsSinceEpoch()) +
                                      " " +
                                      result[0].at(j), redis_config.at(0), redis_config.at(1).toInt());
//                        if (logsheet.at(index).toInt() <= monita_cfg.config.at(2).toInt()) {
                            rds.reqRedis("hset monita_service:temp " +
                                          titik_ukur.at(j) +
                                          "_" +
                                          QString::number(QDateTime::currentMSecsSinceEpoch())
                                            .left(QString::number(QDateTime::currentMSecsSinceEpoch()).length() - 3) +
//                                          QString::number(QDateTime::currentMSecsSinceEpoch()) +
                                          " " +
                                          result[0].at(j), redis_config.at(0), redis_config.at(1).toInt());
                            log.write("Database",
                                      QString::number(j) + " - " +
                                      titik_ukur.at(j) + " - " +
                                      result[0].at(j),
                                      monita_cfg.config.at(6).toInt());
                            logsheet.replace(index, QString::number(logsheet.at(index).toInt()+1));
//                        }
                        rds.reqRedis("hset monita_service:vismon " +
                                     SerialNumber + ";" +
                                     titik_ukur.at(j) +
                                     " " +
                                     QString::number(QDateTime::currentMSecsSinceEpoch())
                                       .left(QString::number(QDateTime::currentMSecsSinceEpoch()).length() - 3) + ";" +
//                                     QString::number(QDateTime::currentMSecsSinceEpoch()) + ";" +
                                     result[0].at(j), redis_config.at(0), redis_config.at(1).toInt());
                        log.write("TcpModbus",
                                  QString::number(j) + " - " +
                                  titik_ukur.at(j) + " - " +
                                  result[0].at(j),
                                  monita_cfg.config.at(6).toInt());
                    }
                }
            }
        }
    }
}
void process::skywave_parse(QJsonObject obj, int index) {
//    QStringList list_arg = arg.split(" ");
//    QStringList list_SinMin = list_arg.at(7).split(",");

    for (int i = 0; i < SN.length(); i++) {
        QStringList titik_ukur;
        QString SerialNumber = SN.at(i);

        QStringList redis_config = cfg.read("REDIS");
        QStringList request = rds.reqRedis("hlen monita_service:parsing_ref", redis_config.at(0), redis_config.at(1).toInt());
        int redis_len = request.at(0).toInt();
        request = rds.reqRedis("hgetall monita_service:parsing_ref", redis_config.at(0), redis_config.at(1).toInt(), redis_len*2);
        for (int j = 0; j < request.length(); j+=2) {
            QStringList temp = request.at(j).split(";");
            if (SerialNumber == temp.at(0)) {
                titik_ukur.insert(temp.at(1).toInt()-1, request.at(j+1));
            }
        }

        if (titik_ukur.length() > 0) {
            QStringList result[3];
            if (obj.value("skywave").isArray()) {
                QJsonArray array = obj.value("skywave").toArray();
                foreach (const QJsonValue & v, array) {
                    if (SerialNumber == v.toObject().value("MobileID").toString()) {
                        result[0].append(v.toObject().value("MobileID").toString());
//                        result.append(v.toObject().value("SIN").toString());
//                        result.append(v.toObject().value("MIN").toString());
                        result[1].append(v.toObject().value("ReceiveUTC").toString());
                        QJsonArray payload = v.toObject().value("Payload").toArray();
                        QString load;
                        foreach (const QJsonValue & vPayLoad, payload) {
                            load = load + vPayLoad.toString() + ";";
                        }
                        result[2].append(load);
                    }
                }
                if (result[0].length() > 0) {
                    for (int j = 0; j < result[0].length(); j++) {
                        if (!result[2].at(j).isEmpty()) {
                            QStringList list_val = result[2].at(j).split(";");
                            if (list_val.length()-1 > titik_ukur.length()) {
                                for (int k = 0; k < titik_ukur.length(); k++) {
                                    rds.reqRedis("hset monita_service:history:" + monita_cfg.config.at(3) +
                                                  titik_ukur.at(k) + " " +
                                                  QString::number(QDateTime::fromString(result[1].at(j), "yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch())
                                                    .left(QString::number(QDateTime::fromString(result[1].at(j), "yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch()).length() - 3) +
//                                                  QString::number(QDateTime::fromString(result[1].at(j), "yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch()) +
                                                  " " +
                                                  list_val.at(k), redis_config.at(0), redis_config.at(1).toInt());

                                    rds.reqRedis("hset monita_service:temp " +
                                                  titik_ukur.at(k) +
                                                  "_" +
                                                  QString::number(QDateTime::fromString(result[1].at(j), "yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch())
                                                    .left(QString::number(QDateTime::fromString(result[1].at(j), "yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch()).length() - 3) +
//                                                  QString::number(QDateTime::fromString(result[1].at(j), "yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch()) +
                                                  " " +
                                                  list_val.at(k), redis_config.at(0), redis_config.at(1).toInt());

                                    rds.reqRedis("hset monita_service:vismon " +
                                                  SerialNumber + ";" +
                                                  titik_ukur.at(k) +
                                                  " " +
                                                  QString::number(QDateTime::fromString(result[1].at(j), "yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch())
                                                    .left(QString::number(QDateTime::fromString(result[1].at(j), "yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch()).length() - 3) + ";" +
//                                                  QString::number(QDateTime::fromString(result[1].at(j), "yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch()) + ";" +
                                                  list_val.at(k), redis_config.at(0), redis_config.at(1).toInt());
                                    log.write("SkyWave",
                                              QString::number(k) + " - " +
                                              result[0].at(j) + " - " +
                                              result[1].at(j) + " - " +
                                              titik_ukur.at(k) + " - " +
                                              list_val.at(k),
                                              monita_cfg.config.at(6).toInt());
                                }
                            } else {
                                for (int k = 0; k < list_val.length()-1; k++) {
                                    rds.reqRedis("hset monita_service:history:" + monita_cfg.config.at(3) +
                                                  titik_ukur.at(k) + " " +
                                                  QString::number(QDateTime::fromString(result[1].at(j), "yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch())
                                                    .left(QString::number(QDateTime::fromString(result[1].at(j), "yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch()).length() - 3) +
                                                  " " +
                                                  list_val.at(k), redis_config.at(0), redis_config.at(1).toInt());

                                    rds.reqRedis("hset monita_service:temp " +
                                                  titik_ukur.at(k) +
                                                  "_" +
                                                  QString::number(QDateTime::fromString(result[1].at(j), "yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch())
                                                    .left(QString::number(QDateTime::fromString(result[1].at(j), "yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch()).length() - 3) +
//                                                  QString::number(QDateTime::fromString(result[1].at(j), "yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch()) +
                                                  " " +
                                                  list_val.at(k), redis_config.at(0), redis_config.at(1).toInt());

                                    rds.reqRedis("hset monita_service:vismon " +
                                                  SerialNumber + ";" +
                                                  titik_ukur.at(k) +
                                                  " " +
                                                  QString::number(QDateTime::fromString(result[1].at(j), "yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch())
                                                    .left(QString::number(QDateTime::fromString(result[1].at(j), "yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch()).length() - 3) + ";" +
//                                                  QString::number(QDateTime::fromString(result[1].at(j), "yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch()) + ";" +
                                                  list_val.at(k), redis_config.at(0), redis_config.at(1).toInt());
                                    log.write("SkyWave",
                                              QString::number(k) + " - " +
                                              titik_ukur.at(k) + " - " +
                                              list_val.at(k),
                                              monita_cfg.config.at(6).toInt());
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (!obj.value("NextStartUTC").isUndefined()) {
        cfg.set_nextutc_skywave(id.at(index),
            QString::number(
                QDateTime::fromString(
                    obj.value("NextStartUTC").toString(), "yyyy-MM-dd HH:mm:ss"
                ).toTime_t()
            )
        );
        QStringList temp = Argv.at(index).split(" ");
        temp.replace(9, obj.value("NextStartUTC").toString().replace(" ","#"));
        Argv.replace(index,
                     temp.at(0) + " " +
                     temp.at(1) + " " +
                     temp.at(2) + " " +
                     temp.at(3) + " " +
                     temp.at(4) + " " +
                     temp.at(5) + " " +
                     temp.at(6) + " " +
                     temp.at(7) + " " +
                     temp.at(8) + " " +
                     temp.at(9) + " " +
                     temp.at(10) + " " +
                     temp.at(11)
                     );
    }
}

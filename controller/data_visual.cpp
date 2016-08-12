#include "data_visual.h"

data_visual::data_visual(QObject *parent) : QObject(parent)
{
//    QTimer *t = new QTimer(this);
    //    connect(t, SIGNAL(timeout()), this, SLOT(doWork());
}

void data_visual::doSetup(QThread &cThread)
{
    connect(&cThread,SIGNAL(started()),this,SLOT(doWork()));

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(doWork()));
    t->start(1000);
}

void data_visual::RedisToJson(QStringList data, QDateTime dt)
{
    monita_cfg.source_config = cfg.read("SOURCE");
    if (monita_cfg.source_config.length() > 7) {monita_cfg.jml_sumber = monita_cfg.source_config.length()/7;
    } else {monita_cfg.jml_sumber++;}

    QJsonObject json;
    QJsonArray slaveArray[monita_cfg.jml_sumber];
    QJsonObject idTitikUkurObject[monita_cfg.jml_sumber];

    QStringList list_temp; QString temp; int index = 0;
    for (int i = 0; i < data.length(); i+=2) {
        list_temp = data.at(i).split(":");
//        QJsonObject dataObject;
//        dataObject["ID_TITIK_UKUR:"+list_temp.at(1)] = data.at(i+1);
//        idTitikUkurObject["ID_TITIK_UKUR:"+list_temp.at(1)] = dataObject;
        if ((i > 0 && temp != list_temp.at(0))) {
            slaveArray[index].append(idTitikUkurObject[index]);
            json["SLAVE_ID:"+temp] = slaveArray[index];
            index++;
        }
        idTitikUkurObject[index]["ID_TITIK_UKUR:"+list_temp.at(1)] = data.at(i+1);
        temp = list_temp.at(0);
        if (i >= data.length()-2) {
            slaveArray[index].append(idTitikUkurObject[index]);
            json["SLAVE_ID:"+temp] = slaveArray[index];
        }
    }

//    QJsonArray redisArray;
//    QJsonObject redisObject;
//    temp = "127.0.0.1";
//    redisObject["IP"] = temp;
//    redisObject["PORT"] = 6379;
//    redisArray.append(redisObject);
//    json["REDIS"] = redisArray;

//    QJsonArray configArray;
//    QJsonObject configObject;
//    configObject["INTERVAL"] = 1000;    //milis
//    configObject["DB_PERIOD"] = 60;     //detik
//    configObject["TIMESTAMP"] = 3;     //TimeStamp
//    temp = ".MonSerConfig/log.txt";
//    configObject["LOG_PATH"] = temp;
//    configArray.append(configObject);
//    json["CONFIG"] = configArray;

    this->WriteToJson(json, dt);
}

void data_visual::WriteToJson(QJsonObject json, QDateTime dt)
{
    QString path = ".MonSerConfig/VisMon_" + dt.date().toString("dd_MM_yyyy") + ".json";
    QFile visual_json_file(path);
    if (!visual_json_file.exists()) {
        QDir dir;
        dir.mkpath(".MonSerConfig");
    }
    if (visual_json_file.open(QIODevice::ReadWrite)) {
        QByteArray readFile = visual_json_file.readAll();
//        if (readFile.isEmpty()) {
            QJsonDocument saveDoc(json);
            visual_json_file.write(saveDoc.toJson());
//        }
    }
}

void data_visual::doWork()
{
    monita_cfg.config = cfg.read("CONFIG");
    QStringList redis_config = cfg.read("REDIS");
    QString address = redis_config.at(0);
    int port = redis_config.at(1).toInt();
    QDateTime dt = QDateTime::currentDateTime();
    QStringList request = rds.reqRedis("hlen vismon", address, port);
    log.write("Redis",request.at(0) + " Data ..");
    int redis_len = request.at(0).toInt();
    request = rds.reqRedis("hgetall vismon", address, port, redis_len*2);

    this->RedisToJson(request, dt);
}


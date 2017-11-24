#include "data_mysql.h"

data_mysql::data_mysql(QObject *parent) : QObject(parent)
{
}

void data_mysql::doSetup(QThread &cThread)
{
    connect(&cThread,SIGNAL(started()),this,SLOT(set_dataHarian()));

    monita_cfg.config = cfg.read("CONFIG");

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(set_dataHarian()));
    t->start(monita_cfg.config.at(0).toInt());

    db = mysql.connect_db("TcpModBus");
}

void data_mysql::set_dataHarian()
{
    monita_cfg.config = cfg.read("CONFIG");
    QStringList redis_config = cfg.read("REDIS");
    QString address = redis_config.at(0);
    int port = redis_config.at(1).toInt();
    QDateTime dt_sdh = QDateTime::currentDateTime();
//    QStringList request = rds.reqRedis("hlen monita_service:data_jaman_" + QDate::currentDate().toString("dd_MM_yyyy"), address, port);
    QStringList request = rds.reqRedis("hlen monita_service:temp", address, port);
//    log.write("Redis",request.at(0) + " Data ..",
//              monita_cfg.config.at(7).toInt());
    if (request.isEmpty()) return;
    int redis_len = request.at(0).toInt();

//    request = rds.reqRedis("hgetall monita_service:data_jaman_" + QDate::currentDate().toString("dd_MM_yyyy"), address, port, redis_len*2);
    request = rds.reqRedis("hgetall monita_service:temp", address, port, redis_len*2);
//    qSort(request.begin(), request.end());
    QStringList temp1; QString data;
//    QStringList temp2; QStringList slave; QStringList tanggal;
    QStringList id_titik_ukur; QStringList data_tunggal; QStringList waktu; QStringList list_waktu;
    for (int i = 0; i < request.length(); i+=2) {
        temp1 = request.at(i).split("_");
//        temp2 = temp1.at(0).split(";");
//        slave.append(temp2.at(0));
        id_titik_ukur.append(temp1.at(0));
//        tanggal.append(temp1.at(1).split("-"));
        waktu.append(temp1.at(1));
        data_tunggal.append(request.at(i+1));

        bool bWaktu = false;
        if (list_waktu.length() > 0) {
            for (int j = 0; j < list_waktu.length(); j++) {
                if (list_waktu.at(j) == temp1.at(1)) {
                    bWaktu = true;
                    break;
                }
            }
        }
        if (!bWaktu) list_waktu.append(temp1.at(1));
    }
//    int t = 0;
    while (!db.isOpen()) {
//        db.close();
        db.open();
        if (!db.isOpen()) {
            mysql.close(db);
            log.write("Database","Error : Connecting Fail ..!!",
                      monita_cfg.config.at(7).toInt());
            QThread::msleep(DELAY_DB_CONNECT);
            db = mysql.connect_db("TcpModBus");
//            t++;
//            if (t >= 3) {emit finish();}
        }
    }
    for (int j = 0; j < list_waktu.length(); j++) {
        dt_sdh = QDateTime::fromTime_t(list_waktu.at(j).toInt());
        data = "";
        if (!get.check_table_is_available(db, monita_cfg.config.at(4) + dt_sdh.date().toString("yyyyMMdd"), "Database", monita_cfg.config.at(7).toInt())) {
            set.create_tabel_data_harian(db, monita_cfg.config.at(4) + dt_sdh.date().toString("yyyyMMdd"), "Database", monita_cfg.config.at(7).toInt());
        }
        for (int i = 0; i < redis_len; i++) {
            if (QDateTime::fromTime_t(waktu.at(i).mid(0, 10).toInt()).toString("yyyyMMdd") ==
                    dt_sdh.date().toString("yyyyMMdd")) {
                data = data + "(" +
                    id_titik_ukur.at(i) + ", " +
//                    id_titik_ukur.at(i) + ", " +
                    data_tunggal.at(i) + ", " +
                    waktu.at(i) +
                        ")";
//                log.write("Debug",id_titik_ukur.at(i));
                if (i != redis_len - 1) {
                    data = data + ",";
                }
            }
        }
        set.data_harian(db, monita_cfg.config.at(4) + dt_sdh.date().toString("yyyyMMdd"), data, "Database", monita_cfg.config.at(7).toInt());
        set.data_harian(db, "data", data, "Database", monita_cfg.config.at(7).toInt());
    }
//    if (db.isOpen()) db.close();
//    mysql.close(db);
    db.close();
    log.write("Database","Data Inserted on table data_" + QDate::currentDate().toString("yyyyMMdd") + " ..",
              monita_cfg.config.at(7).toInt());
//    rds.reqRedis("del data_jaman_" + QDate::currentDate().toString("dd_MM_yyyy"), address, port);
    rds.reqRedis("del monita_service:temp", address, port);
//    qint64 Y_epoch = QDateTime::currentDateTime().toTime_t()-86400;
//    QString yesterday = QDateTime::fromTime_t(Y_epoch).toString("dd_MM_yyyy");
//    rds.reqRedis("del monita_service:" + monita_cfg.config.at(3) + QDateTime::currentDateTime().toString("dd_MM_yyyy"), address, port);
//    rds.reqRedis("del monita_service:" + monita_cfg.config.at(3) + yesterday, address, port);
//    rds.reqRedis("del monita_service:vismon", address, port);
//    mysql.close(db);
//    db.close();
}


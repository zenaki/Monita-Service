#include "data_mysql.h"

data_mysql::data_mysql(QObject *parent) : QObject(parent)
{
}

void data_mysql::doSetup(QThread &cThread)
{
    connect(&cThread,SIGNAL(started()),this,SLOT(set_dataHarian()));

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(set_dataHarian()));
    t->start(60000);

    db = mysql.connect_db();
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
    log.write("Redis",request.at(0) + " Data ..",
              monita_cfg.config.at(8).toInt());
    int redis_len = request.at(0).toInt();

//    request = rds.reqRedis("hgetall monita_service:data_jaman_" + QDate::currentDate().toString("dd_MM_yyyy"), address, port, redis_len*2);
    request = rds.reqRedis("hgetall monita_service:temp", address, port, redis_len*2);
//    qSort(request.begin(), request.end());
    QStringList temp1; QStringList temp2; QString data;
    QStringList slave; QStringList id_titik_ukur;
    QStringList tanggal; QStringList data_tunggal; QStringList waktu;
    for (int i = 0; i < request.length(); i+=2) {
        temp1 = request.at(i).split("_");
        temp2 = temp1.at(0).split(";");
        slave.append(temp2.at(0));
        id_titik_ukur.append(temp2.at(1));
        tanggal.append(temp1.at(1).split("-"));
        waktu.append(temp1.at(2).split(":"));
        data_tunggal.append(request.at(i+1));
    }
//    int t = 0;
    while (!db.isOpen()) {
//        db.close();
        db.open();
        if (!db.isOpen()) {
            mysql.close(db);
            log.write("Database","Error : Connecting Fail ..!!",
                      monita_cfg.config.at(8).toInt());
            QThread::msleep(DELAY_DB_CONNECT);
            db = mysql.connect_db();
//            t++;
//            if (t >= 3) {emit finish();}
        }
    }
    if (!get.check_table_is_available(db, monita_cfg.config.at(4) + dt_sdh.date().toString("yyyyMMdd"), "TcpModbus", monita_cfg.config.at(8).toInt())) {
        set.create_tabel_data_harian(db, monita_cfg.config.at(4) + dt_sdh.date().toString("yyyyMMdd"), "TcpModbus", monita_cfg.config.at(8).toInt());
    }
    for (int i = 0; i < redis_len; i++) {
        data = data + "(" +
            slave.at(i) + id_titik_ukur.at(i) + ", " +
            tanggal.at(i*3+2) + tanggal.at(i*3+1) + tanggal.at(i*3) +
            waktu.at(i*4) + waktu.at(i*4+1) + waktu.at(i*4+2) + waktu.at(i*4+3) + ", " +
            data_tunggal.at(i) + ", " +
            waktu.at(i*4) + ", " +
            waktu.at(i*4+1) + ", " +
            waktu.at(i*4+2) +
                ")";
//        log.write("Debug",id_titik_ukur.at(i));
        if (i != redis_len - 1) {
            data = data + ",";
        }
    }
    set.data_harian(db, monita_cfg.config.at(4) + dt_sdh.date().toString("yyyyMMdd"), data, "TcpModbus", monita_cfg.config.at(8).toInt());
//    if (db.isOpen()) db.close();
//    mysql.close(db);
    db.close();
    log.write("Database","Data Inserted on table data_" + QDate::currentDate().toString("dd_MM_yyyy") + " ..",
              monita_cfg.config.at(8).toInt());
//    rds.reqRedis("del data_jaman_" + QDate::currentDate().toString("dd_MM_yyyy"), address, port);
    rds.reqRedis("del monita_service:temp", address, port);
    rds.reqRedis("del monita_service:" + monita_cfg.config.at(3) + QDate::currentDate().toString("dd_MM_yyyy"), address, port);
    rds.reqRedis("del monita_service:vismon", address, port);
//    mysql.close(db);
//    db.close();
}


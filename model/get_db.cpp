#include "get_db.h"

get_db::get_db()
{
}

bool get_db::check_table_is_available(QSqlDatabase db, QString table_name, QString mode, int debug) {
    if (db.isValid() && db.isOpen()) {
        QString query;
        QSqlQuery q(QSqlDatabase::database(db.connectionName()));

        query.clear();
        query = "SELECT * FROM " + table_name + ";";

        log.write(mode, query, debug);

        q.prepare(query);
        return q.exec();
    } else {
        return false;
    }
}

void get_db::skyWave_config(QSqlDatabase db, monita_config *mon, QString mode, int debug) {
    if (db.isValid()) {
        if (db.isOpen()) {
            db.open();
        }
        QString query;
        QSqlQuery q(QSqlDatabase::database(db.connectionName()));
        QString url;
        mon->jml_gateWay = 0;
        query = "select "
                "gw.id,"
                "gw.url,"
                "gw.access_id,"
                "gw.password,"
                "gw.next_utc,"
                "sh.id_ship,"
                "sh.modem_id,"
                "gw.config,"
                "pr.id_tu "
            "from "
                "gateway gw, "
                "ship sh, "
                "parsing_ref pr "
            "where "
                "gw.id = sh.gateway and "
                "sh.id_ship = pr.id_ship and "
                "gw.status = 1 and "
                "sh.status = 1 and "
                "gw.config is not null and "
                "sh.modem_id is not null and "
                "gw.next_utc is not null and "
                "gw.password is not null and "
                "gw.access_id is not null and "
                "gw.url is not null "
            "order by "
                "gw.url, sh.id_ship, pr.urutan_data_monita;";
        log.write(mode, query, debug);
        q.prepare(query);
        if (q.exec()) {
            while(q.next()) {
                if (mon->jml_gateWay > 0) {
                    url.sprintf("%sget_return_messages.json/?access_id=%s&password=%s&start_utc=",
                                q.value(1).toString().toUtf8().data(),
                                q.value(2).toString().toUtf8().data(),
                                q.value(3).toString().toUtf8().data());
                    if (mon->sky[mon->jml_gateWay-1].id_gateWay == q.value(0).toInt()) {
                        if (mon->sky[mon->jml_gateWay-1].mdm[mon->sky[mon->jml_gateWay-1].jml_modem-1].modem_id == q.value(6).toString()) {
                            mon->sky[mon->jml_gateWay-1].mdm[mon->sky[mon->jml_gateWay-1].jml_modem-1].id_tu.append(q.value(8).toString());
                        } else {
                            mon->sky[mon->jml_gateWay-1].mdm[mon->sky[mon->jml_gateWay-1].jml_modem].id_ship = q.value(5).toInt();
                            mon->sky[mon->jml_gateWay-1].mdm[mon->sky[mon->jml_gateWay-1].jml_modem].modem_id = q.value(6).toString();
                            mon->sky[mon->jml_gateWay-1].mdm[mon->sky[mon->jml_gateWay-1].jml_modem].id_tu.append(q.value(8).toString());
                            mon->sky[mon->jml_gateWay-1].jml_modem++;
                        }
                    } else {
                        mon->sky[mon->jml_gateWay].jml_modem = 0;
                        url.sprintf("%sget_return_messages.json/?access_id=%s&password=%s&start_utc=",
                                    q.value(1).toString().toUtf8().data(),
                                    q.value(2).toString().toUtf8().data(),
                                    q.value(3).toString().toUtf8().data());
                        mon->sky[mon->jml_gateWay].url = url;
                        mon->sky[mon->jml_gateWay].id_gateWay = q.value(0).toInt();
                        mon->sky[mon->jml_gateWay].next_utc = q.value(4).toDateTime();
                        mon->sky[mon->jml_gateWay].mdm[mon->sky[mon->jml_gateWay].jml_modem].id_ship = q.value(5).toInt();
                        mon->sky[mon->jml_gateWay].mdm[mon->sky[mon->jml_gateWay].jml_modem].modem_id = q.value(6).toString();
                        mon->sky[mon->jml_gateWay].SIN_MIN = q.value(7).toString();
                        mon->sky[mon->jml_gateWay].mdm[mon->sky[mon->jml_gateWay].jml_modem].id_tu.append(q.value(8).toString());
                        mon->sky[mon->jml_gateWay].jml_modem++;
                        mon->jml_gateWay++;
                    }
                } else {
                    mon->sky[mon->jml_gateWay].jml_modem = 0;
                    url.sprintf("%sget_return_messages.json/?access_id=%s&password=%s&start_utc=",
                                q.value(1).toString().toUtf8().data(),
                                q.value(2).toString().toUtf8().data(),
                                q.value(3).toString().toUtf8().data());
                    mon->sky[mon->jml_gateWay].url = url;
                    mon->sky[mon->jml_gateWay].id_gateWay = q.value(0).toInt();
                    mon->sky[mon->jml_gateWay].next_utc = q.value(4).toDateTime();
                    mon->sky[mon->jml_gateWay].mdm[mon->sky[mon->jml_gateWay].jml_modem].id_ship = q.value(5).toInt();
                    mon->sky[mon->jml_gateWay].mdm[mon->sky[mon->jml_gateWay].jml_modem].modem_id = q.value(6).toString();
                    mon->sky[mon->jml_gateWay].SIN_MIN = q.value(7).toString();
                    mon->sky[mon->jml_gateWay].mdm[mon->sky[mon->jml_gateWay].jml_modem].id_tu.append(q.value(8).toString());
                    mon->sky[mon->jml_gateWay].jml_modem++;
                    mon->jml_gateWay++;
                }
            }
        }
        db.close();
    }
}

//int get_db::sum_ship(QSqlDatabase db) {
//    QSqlQuery q(db);

//    q.prepare("SELECT COUNT(id_ship) FROM ship");
//    if (!q.exec()){
//        printf("err():\n");
//        return (int) 0;
//    }
//    else{
//        while(q.next()){
//            return (int) q.value(0).toInt();
//        }
//    }
//    return 0;
//}

//int get_db::id_tu_ship(QSqlQuery *q, int id_ship, int urutan){
//    QString query;

//    query.sprintf("SELECT count(id_tu), id_tu FROM parsing_ref WHERE id_ship = %d and urutan_data_monita = %d", id_ship, urutan);

//#if 1
//    q->prepare(query);
//    if(!q->exec()){
//        printf("err():\n");
//        return (int) 0;
//    }
//    else{
//        while(q->next()){
//            if (q->value(0).toInt() == 0){
//                return(int) 0;
//            }
//            else{
//                return (int) q->value(1).toInt();
//            }
//        }
//    }
//#endif
//    return 0;
//}

void get_db::modem_info(QSqlDatabase db, sky_wave_ship *marine){
    db.open();
    QSqlQuery q(QSqlDatabase::database(db.connectionName()));

    int count = 0;

    q.prepare("SELECT id_ship, name, modem_id FROM ship where status = 1");
    if(!q.exec()){
        return;
    }
    else{
        while(q.next()){
            marine->kapal[count].id_ship =  q.value(0).toInt();
            strcpy(marine->kapal[count].name, q.value(1).toString().toLatin1());
            strcpy(marine->kapal[count].modem_id, q.value(2).toString().toLatin1());

            count++;
        }
    }

    marine->sum_ship = count;

    db.close();
    return;
}

void get_db::modem_getway(QSqlDatabase db, sky_wave_account *acc){
    db.open();
    QSqlQuery q(QSqlDatabase::database(db.connectionName()));

    int n = 0;

    q.prepare("SELECT id, url, access_id, password, next_utc, SIN, MIN, status from gateway");
    if(!q.exec()){
        return;
    }
    else{
        while(q.next()){
             QString qStr;

             int status_active = q.value((7)).toInt();

             if(status_active){
                 int id = q.value(0).toInt();
                 QString getway = q.value(1).toString();
                 QString access_id = q.value(2).toString();
                 QString password = q.value(3).toString();
                 QDateTime nextutc = q.value(4).toDateTime();
                 int SIN = q.value(5).toInt();
                 int MIN = q.value(6).toInt();

//                 qStr.sprintf("%sget_return_messages.xml/?access_id=%s&password=%s&start_utc=",
//                              getway.toUtf8().data(), access_id.toUtf8().data(), password.toUtf8().data());
                 qStr.sprintf("%sget_return_messages.json/?access_id=%s&password=%s&start_utc=",
                              getway.toUtf8().data(), access_id.toUtf8().data(), password.toUtf8().data());

                 acc->gway[n].id = id;
                 strcpy(acc->gway[n].link, qStr.toLatin1());
                 strcpy(acc->gway[n].nextutc, nextutc.toString("yyyy-MM-dd%20hh:mm:ss").toUtf8().data());
                 strcpy(acc->gway[n].access_id, access_id.toLatin1());
                 strcpy(acc->gway[n].password, password.toLatin1());
                 acc->gway[n].SIN = SIN;
                 acc->gway[n].MIN = MIN;
                 acc->gway[n].status = status_active;

                 n++;
             }
        }
    }

    acc->sum_getway = n;

    db.close();
    return;
}

//bool get_db::check_available_table(QSqlQuery *q, int index){
//    QString query;

//    query.clear();
//    query.sprintf("select * from data_%d", index);

//    q->prepare(query);

//    return q->exec();
//}

#include "get_db.h"

get_db::get_db()
{
}

bool get_db::check_table_is_available(QSqlDatabase db, QString table_name) {
    QString query;
    QSqlQuery q(db);

    query.clear();
    query = "SELECT * FROM " + table_name + ";";

    q.prepare(query);

    return q.exec();
}

QStringList get_db::get_config(QSqlDatabase db) {
    db.open();
    QSqlQuery q(db);
    QStringList list; int count;
    q.prepare("SELECT ip_address, port, slave_id, function_code, start_address, num_of_coils FROM config where id_config = 1 and status = 1");
    if(!q.exec()){
        return list;
    }
    else{
        do {
            for (int i = 0; i < 6; i++) {
                if (!q.isNull(i)) {
                    list.insert(count, q.value(i).toString());
                    count++;
                }
            }
        } while(q.next());
    }
    db.close();
    return list;
}

int get_db::sum_ship(QSqlDatabase db) {
    QSqlQuery q(db);

    q.prepare("SELECT COUNT(id_ship) FROM ship");
    if (!q.exec()){
        printf("err():\n");
        return (int) 0;
    }
    else{
        while(q.next()){
            return (int) q.value(0).toInt();
        }
    }
}

int get_db::id_tu_ship(QSqlQuery *q, int id_ship, int urutan){
    QString query;

    query.sprintf("SELECT count(id_tu), id_tu FROM parsing_ref WHERE id_ship = %d and urutan_data_monita = %d", id_ship, urutan);

#if 1
    q->prepare(query);
    if(!q->exec()){
        printf("err():\n");
        return (int) 0;
    }
    else{
        while(q->next()){
            if (q->value(0).toInt() == 0){
                return(int) 0;
            }
            else{
                return (int) q->value(1).toInt();
            }
        }
    }
#endif
}

void get_db::modem_info(QSqlDatabase db, sky_wave_ship *marine){
    db.open();
    QSqlQuery q(db);

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
    QSqlQuery q(db);

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

                 qStr.sprintf("%sget_return_messages.xml/?access_id=%s&password=%s&start_utc=",
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

bool get_db::check_available_table(QSqlQuery *q, int index){
    QString query;

    query.clear();
    query.sprintf("select * from data_%d", index);

    q->prepare(query);

    return q->exec();
}

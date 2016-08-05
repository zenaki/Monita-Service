#ifndef SAVE_DB_H
#define SAVE_DB_H

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <QDateTime>

class save_db
{
public:
    save_db();

    void data_harian(QSqlDatabase db, QString tanggal, QString data);
    void create_tabel_data_harian(QSqlDatabase db, QString tb_name, QString tanggal);
    void create_tabel_data_harian_skywave(QSqlQuery *q, int index);
    void data_skywave(QSqlQuery *q, float value, int id_tu, int id_trip, int epochtime, QString data_time, int flag);
    void data_harian_skywave(QSqlQuery *q, float value, int id_tu, int id_trip, int epochtime, QString data_time, int index, int flag);
    void update_next_utc_skywave(QSqlDatabase db, QString next_utc, int id_ship);
    void update_next_utc_gateway_skywave(QSqlDatabase db, QString next_utc, int id_gateway);
};

#endif // SAVE_DB_H

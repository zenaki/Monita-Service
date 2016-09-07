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

    void data_harian(QSqlDatabase db, QString tb_name, QString data);
    void create_tabel_data_harian(QSqlDatabase db, QString tb_name);

    void create_table_data_punya_skywave(QSqlDatabase db, QString tb_name);
    void data_punya_skywave(QSqlDatabase db, QString tb_name, QString data);
    void update_multiple_row_punya_skywave(QSqlDatabase db, QString tb_name, QString col_target, QString col_clause, QString data1, QString data2);
    void delete_data_jaman_punya_skywave(QSqlDatabase db, QString tb_name, int epochtime);

    void create_tabel_data_skywave(QSqlQuery *q);
    void create_tabel_data_harian_skywave(QSqlQuery *q, int index);
    void data_skywave(QSqlQuery *q, float value, int id_tu, int id_trip, int epochtime, QString data_time, int flag);
    void data_harian_skywave(QSqlQuery *q, float value, int id_tu, int id_trip, int epochtime, QString data_time, int index, int flag);
    void update_next_utc_skywave(QSqlDatabase db, QString next_utc, int id_ship);
    void update_next_utc_gateway_skywave(QSqlDatabase db, QString next_utc, int id_gateway);
};

#endif // SAVE_DB_H

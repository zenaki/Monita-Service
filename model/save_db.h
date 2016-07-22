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
    void create_tabel_data_harian(QSqlDatabase db, QString tanggal);
};

#endif // SAVE_DB_H

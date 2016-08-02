#ifndef GET_DB_H
#define GET_DB_H

#include <util/utama.h>

class get_db
{
public:
    get_db();

    bool check_table_is_available(QSqlDatabase db, QString table_name);
    QStringList get_config(QSqlDatabase db);

    ///** SKY WAVE **///
    int sum_ship(QSqlDatabase db);
    int id_tu_ship(QSqlQuery *q, int id_ship, int urutan);
    void modem_info(QSqlDatabase db, struct sky_wave_ship *marine);
    void modem_getway(QSqlDatabase db, struct sky_wave_account *acc);
    bool check_available_table(QSqlQuery *q, int index);

    init_mysql mysql;
    monita_log mlog;
};

#endif // GET_DB_H

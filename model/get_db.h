#ifndef GET_DB_H
#define GET_DB_H

#include <util/utama.h>

class get_db
{
public:
    get_db();

    monita_log log;

    bool check_table_is_available(QSqlDatabase db, QString table_name, QString type, int debug);
    void skyWave_config(QSqlDatabase db, struct monita_config *mon, QString type, int debug);

    ///** SKY WAVE **///
    int sum_ship(QSqlDatabase db);
    int id_tu_ship(QSqlQuery *q, int id_ship, int urutan);
    void modem_info(QSqlDatabase db, struct sky_wave_ship *marine);
    void modem_getway(QSqlDatabase db, struct sky_wave_account *acc);
    bool check_available_table(QSqlQuery *q, int index);

    init_mysql mysql;
};

#endif // GET_DB_H

#ifndef GET_DB_H
#define GET_DB_H

#include <util/utama.h>

class get_db
{
public:
    get_db();

    bool check_table_is_available(QSqlDatabase db, QString table_name);
    QStringList get_config(QSqlDatabase db);

    init_mysql mysql;
    monita_log mlog;
};

#endif // GET_DB_H

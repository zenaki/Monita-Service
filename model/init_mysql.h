#ifndef INIT_MYSQL_H
#define INIT_MYSQL_H

#include <QDebug>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDir>
#include <QFile>
#include <QSettings>

#include "util/utama.h"

class init_mysql
{
public:
    init_mysql();

    QString host;
    QString db_name;
    QString user_name;
    QString password;

    QSqlDatabase connect_db();
};

#endif // INIT_MYSQL_H

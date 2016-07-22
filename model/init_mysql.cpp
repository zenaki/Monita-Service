#include "init_mysql.h"

init_mysql::init_mysql()
{
}

QSqlDatabase init_mysql::connect_db(){

    QFile db_path(PATH_DB_CONNECTION);
    if (db_path.exists()) {
        QSettings db_sett(PATH_DB_CONNECTION, QSettings::IniFormat);
        host = db_sett.value("HOST").toString();
        db_name = db_sett.value("DATABASE").toString();
        user_name = db_sett.value("USERNAME").toString();
        password = db_sett.value("PASSWORD").toString();

        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
        db.setHostName(host);
        db.setDatabaseName(db_name);
        db.setUserName(user_name);
        db.setPassword(password);

        return (QSqlDatabase) db;
    } else {
        host = "localhost";
        db_name = "monita_service";
        user_name = "root";
        password = "root";

        QSettings db_sett(PATH_DB_CONNECTION, QSettings::IniFormat);
        db_sett.setValue("HOST", host.toUtf8());
        db_sett.setValue("DATABASE", db_name.toUtf8());
        db_sett.setValue("USERNAME", user_name.toUtf8());
        db_sett.setValue("PASSWORD", password.toUtf8());

        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
        db.setHostName(host);
        db.setDatabaseName(db_name);
        db.setUserName(user_name);
        db.setPassword(password);

        return (QSqlDatabase) db;
    }
}

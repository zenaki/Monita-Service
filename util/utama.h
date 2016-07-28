#ifndef UTAMA_H
#define UTAMA_H

//#include <QDebug>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QThread>

#include <util/redis.h>
#include <model/save_db.h>
#include <model/init_mysql.h>
#include <model/monita_log.h>

#define DELAY_DB_CONNECT 5000

#define PATH_DB_CONNECTION "monita_configuration/monita-db.dbe"
#define PATH_CONFIGURATION_JSON "monita_configuration/monita-cfg.json"
#define PATH_CONFIGURATION_BIN "monita_configuration/monita-cfg.dat"

struct monita_config {
    QString str_log;
    int modbus_period;
    int jml_sumber;
    QStringList config;
    QStringList source_config;
    QStringList redis_config;
};

#endif // UTAMA_H

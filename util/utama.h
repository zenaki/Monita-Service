#ifndef UTAMA_H
#define UTAMA_H

//#include <QDebug>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QXmlStreamReader>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QDir>
#include <QFile>
#include <QSettings>
#include <QDateTime>
#include <QTimer>
#include <QThread>
#include <QUrl>

#include <model/monita_log.h>
#include <util/redis.h>
#include <model/save_db.h>
#include <model/init_mysql.h>

#define  MODEM_KURAYGEO   1
#define  MODEM_IMANIPRIMA 2
#define  DATA_TRACKING    "noEIO"
#define  FLAG_ENGINE_DATA      0
#define  FLAG_TRACKING_DATA    1
#define  MAX_MEASUREMENT_POINT 999

#define DELAY_DB_CONNECT 5000
#define PATH_DB_CONNECTION ".MonSerConfig/monita-db.dbe"
#define PATH_CONFIGURATION_JSON ".MonSerConfig/monita-cfg.json"
#define PATH_CONFIGURATION_BIN ".MonSerConfig/monita-cfg.dat"

#define JUMLAH_MAX_TITIK_UKUR 10000

struct ship {
    int id_ship;
    char name[32];
    char modem_id[32];
};

struct sky_wave_ship {
    int sum_ship;
    struct ship kapal[50]; /* masih hardoce untuk alokasi 50 kapal */
};

struct getway {
    int id;
    char link[512];
    char nextutc[32];
    char access_id[16];
    char password[16];
    int SIN;
    int MIN;
    int status;
};

struct sky_wave_account {
    int sum_getway;
    struct getway gway[10];
};

struct modem {
    QString modem_id;
    QString last_utc;
    QStringList id_tu;
    QStringList val_tu;
    QString query;
};

struct sky_wave_config {
    QString url;
    QDateTime next_utc;
//    int SIN;
//    int MIN;
    QString SIN_MIN;
    int jml_modem;
    struct modem mdm[20];
};

struct monita_config {
    QString str_log;
    int modbus_period;
    int jml_sumber;
    QStringList config;
    QStringList source_config;
    QStringList sky_config;
    QStringList redis_config;
    QStringList funct_config;

    QString urls;
    int gateway_count;
    int jml_gateWay;
    struct sky_wave_config sky[10];

};

#endif // UTAMA_H

#ifndef SKY_WAVE_H
#define SKY_WAVE_H

#include <QObject>

#include "util/utama.h"
#include "util/config.h"
#include "util/util_skyw.h"
#include "util/parsing_fuction.h"

class sky_wave : public QObject
{
    Q_OBJECT
public:
    explicit sky_wave(QObject *parent = 0);

    config cfg;
    redis rds;
    monita_log log;
    init_mysql mysql;
    struct monita_config monita_cfg;

    void doSetup(QThread &cThread);

private:
    QSqlDatabase db;
    QNetworkAccessManager *manager;
    util_skyw read;
    get_db get;
    save_db set;
    parsing_function parse;

    struct sky_wave_ship *marine;
    struct sky_wave_account *acc;

    int ship_count;
    int gateway_count;
    int cnt_panggil;
    QJsonDocument JsonDoc;

    void parsing(QByteArray data_json, int indexGateWay);
    QStringList parsingRawPayload(QString RawData);

signals:

public slots:
    void doWork();

private slots:
    void replyFinished(QNetworkReply *reply);
};

#endif // SKY_WAVE_H

#ifndef UTIL_SKYW_H
#define UTIL_SKYW_H

#include <QObject>
#include <QXmlStreamReader>
#include <QDebug>
#include <QDir>

#include <QSqlDatabase>
#include <QSqlQuery>

#include <model/get_db.h>
#include <model/save_db.h>
#include <util/parsing_fuction.h>
#include <util/utama.h>

class util_skyw : public QObject
{
    Q_OBJECT
public:
    explicit util_skyw(QObject *parent = 0);

#if 0
    void parse_xml(QString skyw, QSqlQuery *q, int id_ship, int SIN, int MIN, struct utama *kapal, int urut);
#endif

    void parse_xml_account_methode(QString skyw, QSqlDatabase db, struct sky_wave_ship *marine, struct sky_wave_account *acc, int id_gateway, int index_gway);
    void parse_kureyGeo(QString skyw, QSqlDatabase db, struct sky_wave_ship *marine, struct sky_wave_account *acc, int id_gateway, int index_gway);
    void parse_imaniPrima(QString skyw, QSqlDatabase db, struct sky_wave_ship *marine, struct sky_wave_account *acc, int id_gateway, int index_gway);

signals:
    
public slots:

private:
    get_db get;
    save_db save;
    parsing_function parse;

    QString name_df[5];
    float dat_f[5];
    int tu_df[5];
    int cnt_df;

    int jum_dat;

    char *buf_lay;
    char *buf_dateTime;

    void write(QFile *file, const char *text, ...);
};

#endif // UTIL_SKYW_H

#ifndef WORKER_H
#define WORKER_H

#include <QObject>

#include "util/utama.h"
#include "util/config.h"
#include <util/util_skyw.h>

#include "controller/tcp_modbus.h"
#include "controller/data_mysql.h"
#include "controller/data_visual.h"

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = 0);
    ~Worker();

private slots:
    void doWork();
    void replyFinished(QNetworkReply* reply);           /** veranda **/

private:
    config cfg;
    monita_log log;
    QSqlDatabase db;                                    /** veranda **/
    QNetworkAccessManager *manager;                     /** veranda **/
    util_skyw read;                                     /** veranda **/

    tcp_modbus obj_tcp_modbus;
    QThread ThreadTcpModbus;
    data_mysql obj_data_mysql;
    QThread ThreadDataMysql;
    data_visual obj_data_visual;
    QThread ThreadDataVisual;

    struct sky_wave_ship *marine;                       /** veranda **/
    struct sky_wave_account *acc;                       /** veranda **/
    struct monita_config monita_cfg;

    int ship_count;                                     /** veranda **/
    int gateway_count;                                  /** veranda **/
    int cnt_panggil;                                    /** veranda **/

    void request_sky_wave();                            /** veranda **/
};

#endif

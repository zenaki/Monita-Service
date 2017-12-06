#ifndef WORKER_H
#define WORKER_H

#include <QObject>

#include "util/utama.h"
#include "util/config.h"

#include "controller/data_mysql.h"
#include "controller/data_visual.h"
#include "controller/http_server.h"

#include "controller/process.h"

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = 0);
    ~Worker();

private slots:
    void doWork();

private:
    config cfg;
    monita_log log;

    struct plugins plg;
    struct monita_config monita_cfg;
    
    int jml_thread;
    process obj_app[MAX_PLUGINS];
    QThread ThreadApp[MAX_PLUGINS];

    data_mysql obj_data_mysql;
    QThread ThreadDataMysql;
    data_visual obj_data_visual;
    QThread ThreadDataVisual;
    http_server obj_http_server;
    QThread ThreadHttpServer;
};

#endif

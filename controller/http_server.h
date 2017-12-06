#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <QObject>

#include "util/utama.h"
#include "util/config.h"

#include "qttpserver"
#include "util/api.h"

class http_server : public QObject
{
    Q_OBJECT
public:
    explicit http_server(QObject *parent = 0);

    void doSetup(QThread &cThread);

    config cfg;
    redis rds;
    monita_log log;
    struct monita_config monita_cfg;

public slots:
    void doWork();

};

#endif // HTTP_SERVER_H

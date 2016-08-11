#ifndef DATA_MYSQL_H
#define DATA_MYSQL_H

#include <QObject>

#include "util/utama.h"
#include "util/config.h"
#include "model/get_db.h"

class data_mysql : public QObject
{
    Q_OBJECT
public:
    explicit data_mysql(QObject *parent = 0);

    void doSetup(QThread &cThread);

private:
    config cfg;
    redis rds;
    monita_log log;
    init_mysql mysql;
    get_db get;
    save_db set;
    QSqlDatabase db;
    struct monita_config monita_cfg;

public slots:
    void set_dataHarian();
};

#endif // DATA_MYSQL_H

#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QThread>

#include <QSqlQuery>

#include "util/config.h"
#include "util/utama.h"
#include "model/get_db.h"
#include "model/save_db.h"
#include "model/init_mysql.h"
#include "model/monita_log.h"

#include <qmath.h>
#include <errno.h>

#include "imodbus.h"

extern "C"
{
    #include "hiredis/hiredis.h"
}

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = 0);

signals:
    void finish();

public slots:

protected:
    void releaseTcpModbus();
    void connectTcpModbus(const QString &address, int portNbr);

private slots:
    void doWork();
    void resetStatus();
    void pollForDataOnBus();

private:
    config cfg;
    modbus_t *m_tcpModbus;
    int modbus_period;
    int jml_sumber;
    QStringList Modbus_Config;

//    struct config *cfg;
    QStringList list_config;
//    QStringList period_config;

    void request(int index);
    void set_dataHarian(QString address, int port);

    int stringToHex(QString s);
    QString embracedString(const QString s);
    QString descriptiveDataTypeName(int funcCode);

    QStringList reqRedis(QString command, QString address, int port, int len = 0);
    redisReply *r_reply;
    redisContext *r_context;

    /** log **/
    QString fileName;
    QFile *files;
    QTextStream *outStreams;

    init_mysql mysql;
    QTimer timer;
    get_db get;
    save_db set;
    monita_log mlog;

    QSqlDatabase db;
    QSqlQuery *qsql;
};

#endif // TIMER_H

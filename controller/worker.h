#ifndef WORKER_H
#define WORKER_H

#include <QObject>

#include "util/utama.h"
#include "util/config.h"
#include "model/get_db.h"
#include <util/util_modbus.h>
#include <util/util_skyw.h>

#include <qmath.h>
#include <errno.h>

#include "imodbus.h"

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = 0);

signals:
    void finish();

protected:
    void releaseTcpModbus();
    void connectTcpModbus(const QString &address, int portNbr);

private slots:
    void doWork();
    void resetStatus();
    void pollForDataOnBus();
    void replyFinished(QNetworkReply* reply);

private:
    config cfg;
    redis rds;
    monita_log log;
    QTimer timer;
    init_mysql mysql;
    get_db get;
    save_db set;
    QSqlDatabase db;
    modbus_t *m_tcpModbus;
    util_modbus mod;
    QNetworkAccessManager *manager;
    util_skyw read;

    struct sky_wave_ship *marine;
    struct sky_wave_account *acc;
    struct monita_config monita_cfg;

    void request_modbus(int index);
    int stringToHex(QString s);
    QString embracedString(const QString s);
    QString descriptiveDataTypeName(int funcCode);
    void set_dataHarian(QString address, int port);
    void request_sky_wave();
};

#endif // TIMER_H

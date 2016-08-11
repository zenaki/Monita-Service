#ifndef TCP_MODBUS_H
#define TCP_MODBUS_H

#include <QObject>

#include "util/utama.h"
#include "util/config.h"

#include <qmath.h>
#include <errno.h>

#include "imodbus.h"

class tcp_modbus : public QObject
{
    Q_OBJECT
public:
    explicit tcp_modbus(QObject *parent = 0);

    modbus_t *m_tcpModbus;
    config cfg;
    redis rds;
    monita_log log;
    struct monita_config monita_cfg;
    QStringList calc_temp;

    void doSetup(QThread &cThread);

private:
    void request_modbus(int index, QDateTime dt_req_mod);
    int stringToHex(QString s);
    QString embracedString(const QString s);
    QString descriptiveDataTypeName(int funcCode);

    void calculation(int slave_id, int reg, float data, bool logsheet, QDateTime dt_calc);
    QString funct_sum(int id, int reg, QStringList calc_list, float data);
    QString funct_ave(int id, int reg, QStringList calc_list, float data, int jml);
    QString funct_mul(int id, int reg, QStringList calc_list, float data);
    QString funct_min(int id, int reg, QStringList calc_list, float data);
    QString funct_max(int id, int reg, QStringList calc_list, float data);

public slots:
    void doWork();

private slots:
    void resetStatus();
    void pollForDataOnBus();

protected:
    void releaseTcpModbus();
    void connectTcpModbus(const QString &address, int portNbr);
};

#endif // TCP_MODBUS_H

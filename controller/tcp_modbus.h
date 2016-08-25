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
    QStringList funct_temp;

    void doSetup(QThread &cThread);

private:
    void request_modbus(int index, QDateTime dt_req_mod);
    int stringToHex(QString s);
    QString embracedString(const QString s);
    QString descriptiveDataTypeName(int funcCode);

    bool logsheet;

    void LuaRedis_function(QDateTime dt_lua);
    void send_ResultToRedis(QStringList result_data, QDateTime dt_result);
    QByteArray readLua(QString pth);

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

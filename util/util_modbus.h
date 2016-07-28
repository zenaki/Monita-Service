#ifndef UTIL_MODBUS_H
#define UTIL_MODBUS_H

#include "util/utama.h"
#include "util/config.h"

#include <QTimer>

#include <qmath.h>
#include <errno.h>

#include "imodbus.h"

class util_modbus
{
public:
    util_modbus();

    modbus_t *m_tcpModbus;
    monita_log log;
    config cfg;
    struct monita_config monita_cfg;

    void releaseTcpModbus();
    void connectTcpModbus(const QString &address, int portNbr);

    void resetStatus();
    void pollForDataOnBus();

    void get_source();
};

#endif // UTIL_MODBUS_H

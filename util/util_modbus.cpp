#include "util_modbus.h"

util_modbus::util_modbus()
{

}

void util_modbus::releaseTcpModbus()
{
    if( m_tcpModbus ) {
        modbus_close( m_tcpModbus );
        modbus_free( m_tcpModbus );
        m_tcpModbus = NULL;
    }
}

void util_modbus::connectTcpModbus(const QString &address, int portNbr)
{
    releaseTcpModbus();

    m_tcpModbus = modbus_new_tcp( address.toLatin1().constData(), portNbr );
    get_source();
    if( modbus_connect( m_tcpModbus ) == -1 )
    {
        log.write(monita_cfg.config.at(2),"TcpModbus",address + ":" + QString::number(portNbr) + " Could not connect ..");
        releaseTcpModbus();
    } else {
        log.write(monita_cfg.config.at(2),"TcpModbus",address + ":" + QString::number(portNbr) + " Connected ..");
    }
}

void util_modbus::resetStatus()
{
    log.write(monita_cfg.config.at(2),"TcpModbus","Reset : Ready ..");
}

void util_modbus::pollForDataOnBus()
{
    if(m_tcpModbus)
    {
        modbus_poll(m_tcpModbus);
    }
}

void util_modbus::get_source()
{
    monita_cfg.source_config = cfg.read("source");
    if (monita_cfg.source_config.length() > 7) {monita_cfg.jml_sumber = monita_cfg.source_config.length()/7;
    } else {monita_cfg.jml_sumber++;}
}

#include "tcp_modbus.h"

tcp_modbus::tcp_modbus(QObject *parent) : QObject(parent)
{
}

void tcp_modbus::doSetup(QThread &cThread)
{
    connect(&cThread,SIGNAL(started()),this,SLOT(doWork()));

    monita_cfg.jml_sumber = 0;
    monita_cfg.source_config = cfg.read("SOURCE");
    if (monita_cfg.source_config.length() > 9) {monita_cfg.jml_sumber = monita_cfg.source_config.length()/9;
    } else {if (monita_cfg.source_config.length() > 0) monita_cfg.jml_sumber++;}

    monita_cfg.config = cfg.read("CONFIG");
    QStringList redis_config = cfg.read("REDIS");
    rds.reqRedis("del monita_service:vismon", redis_config.at(0), redis_config.at(1).toInt());

    monita_cfg.modbus_period = 0;

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(doWork()));
    t->start(1000);
}

void tcp_modbus::doWork()
{
    QDateTime dateTime = QDateTime::currentDateTime();

    monita_cfg.jml_sumber = 0;
    monita_cfg.source_config = cfg.read("SOURCE");
    if (monita_cfg.source_config.length() > 9) {monita_cfg.jml_sumber = monita_cfg.source_config.length()/9;
    } else {if (monita_cfg.source_config.length() > 0) monita_cfg.jml_sumber++;}

    for (int i = 0; i < monita_cfg.jml_sumber; i++) {
        if (monita_cfg.source_config.at(6) == "TCP") {
            m_tcpModbus = NULL;
            this->connectTcpModbus(monita_cfg.source_config.at(i*9), monita_cfg.source_config.at(i*9+1).toInt());
            if (!m_tcpModbus) {
                monita_cfg.jml_sumber = 0;
                monita_cfg.source_config = cfg.read("SOURCE");
                if (monita_cfg.source_config.length() > 9) {monita_cfg.jml_sumber = monita_cfg.source_config.length()/9;
                } else {if (monita_cfg.source_config.length() > 0) monita_cfg.jml_sumber++;}

                this->connectTcpModbus(monita_cfg.source_config.at(i*9), monita_cfg.source_config.at(i*9+1).toInt());
                if (m_tcpModbus) {
                    this->request_modbus(i, dateTime);
                    releaseTcpModbus();
                } else {
                    QStringList redis_config = cfg.read("REDIS");
                    const int slave = monita_cfg.source_config.at(i*9+2).toInt();
                    const int addr = monita_cfg.source_config.at(i*9+4).toInt();
                    int num = monita_cfg.source_config.at(i*9+5).toInt();
                    for( int i = 0; i < num; ++i ) {
                        rds.reqRedis("hset monita_service:vismon " +
                                       QString::number(slave) + ";" +
                                       QString::number(addr+i) +
                                       " " +
                                       "N/A", redis_config.at(0), redis_config.at(1).toInt());
                    }
                }
                releaseTcpModbus();
            } else {
                this->request_modbus(i, dateTime);
                releaseTcpModbus();
            }
            monita_cfg.modbus_period++;
        }
    }
    this->LuaRedis_function(dateTime);
}

void tcp_modbus::request_modbus(int index, QDateTime dt_req_mod)
{
    if( m_tcpModbus == NULL )
    {
        return;
    }

    const int slave = monita_cfg.source_config.at(index*9+2).toInt();
    const int func = monita_cfg.source_config.at(index*9+3).toInt();
    const int addr = monita_cfg.source_config.at(index*9+4).toInt();
    int num = monita_cfg.source_config.at(index*9+5).toInt();
    const int byte = monita_cfg.source_config.at(index*9+7).toInt();
    QString type = monita_cfg.source_config.at(index*9+8);

    uint8_t dest[1024];
    uint16_t *dest16 = (uint16_t *) dest;

    memset( dest, 0, 1024 );

    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
//    const QString dataType = descriptiveDataTypeName(func);

    modbus_set_slave( m_tcpModbus, slave );
    log.write("TcpModbus", "Request from : " +
              monita_cfg.source_config.at(index*9) + ":" + monita_cfg.source_config.at(index*9+1),
              monita_cfg.config.at(7).toInt());
    switch( func )
    {
        case MODBUS_FC_READ_COILS:
            ret = modbus_read_bits( m_tcpModbus, addr, num, dest );
            break;
        case MODBUS_FC_READ_DISCRETE_INPUTS:
            ret = modbus_read_input_bits( m_tcpModbus, addr, num, dest );
            break;
        case MODBUS_FC_READ_HOLDING_REGISTERS:
            ret = modbus_read_registers( m_tcpModbus, addr, num, dest16 );
            is16Bit = true;
            break;
        case MODBUS_FC_READ_INPUT_REGISTERS:
            ret = modbus_read_input_registers( m_tcpModbus, addr, num, dest16 );
            is16Bit = true;
            break;
//        case MODBUS_FC_WRITE_SINGLE_COIL:
//            ret = modbus_write_bit( m_tcpModbus, addr,
//                    ui->regTable->item(0,2)->text().toInt(0,0)?1:0);
//            writeAccess = true;
//            num = 1;
//            break;
//        case MODBUS_FC_WRITE_SINGLE_REGISTER:
//            ret = modbus_write_register( m_tcpModbus, addr,
//                    ui->regTable->item(0,2)->text().toInt(0,0));
//            writeAccess = true;
//            num = 1;
//            break;
//        case MODBUS_FC_WRITE_MULTIPLE_COILS:
//        {
//            uint8_t * data = new uint8_t[num];
//            for( int i = 0; i < num; ++i )
//            {
//                data[i] = ui->regTable->item(i,2)->text().toInt(0,0);
//            }
//            ret = modbus_write_bits( m_tcpModbus, addr, num, data );
//            delete[] data;
//            writeAccess = true;
//            break;
//        }
//        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
//        {
//            uint16_t * data = new uint16_t[num];
//            for( int i = 0; i < num; ++i )
//            {
//                data[i] = ui->regTable->item(i,2)->text().toInt(0,0);
//            }
//            ret = modbus_write_registers( m_tcpModbus, addr, num, data );
//            delete[] data;
//            writeAccess = true;
//            break;
//        }
        default:
            break;
    }

    if( ret == num  )
    {
        if( writeAccess )
        {
//            printf("Monita::TcpModbus::Value successfully sent ..\n");
            log.write("TcpModbus","Value successfully sent ..", monita_cfg.config.at(7).toInt());
            QTimer::singleShot( 2000, this, SLOT( resetStatus() ) );
        }
        else
        {
//            QString qs_num;
            QString data_int;
            QString data_hex;
            int data_before;
            QByteArray array;
            QString data_real;
            QString data_temp1; QString data_temp2;
            logsheet = false;

            for( int i = 1; i < num+1; ++i )
            {
                int data = is16Bit ? dest16[(i-1)] : dest[(i-1)];
                data_int.sprintf("%d", data);
                data_hex.sprintf("0x%04x", data);
                if (i % byte == 0) {
                    if (byte == 1) {
                        data_temp2.sprintf("%04x", data);
                    } else if (byte == 2) {
                        data_temp2.sprintf("%04x%04x", data_before, data);
                    } else if (byte == 4) {
                        data_temp1.sprintf("%04x", data);
                        data_temp2.append(data_temp1);
                    }
                    bool ok;
                    if (type == "FLOAT") {
//                        int sign = 1;
//                        array = data_temp2.toUtf8();
//                        array = QByteArray::number(array.toLongLong(&ok,16),2); //convert hex to binary -you don't need this since your incoming data is binary
//                        if(array.length()==32) {
//                            if(array.at(0)=='1')  sign =-1;                       // if bit 0 is 1 number is negative
//                            array.remove(0,1);                                     // remove sign bit
//                        }
//                        QByteArray fraction =array.right(23);   //get the fractional part
//                        double mantissa = 0;
//                        for(int i=0;i<fraction.length();i++)     // iterate through the array to claculate the fraction as a decimal.
//                            if(fraction.at(i)=='1')     mantissa += 1.0/(pow(2,i+1));
//                        int exponent = array.left(array.length()-23).toLongLong(&ok,2)-127;     //claculate the exponent
                        unsigned int d_hex = data_temp2.toUInt(&ok, 16);
                        float data_float = (*(float *) &d_hex);
//                        QString data_real2 = QString::number( sign*pow(2,exponent)*(mantissa+1.0),'f', 5 );
//                        QString data_real1 = QString::number(data_float, 'f', 5);
                        data_real = QString::number(data_float, 'f', 5);
                        data_before = 0;
                        data_temp2.clear();
                    } else if (type == "DEC") {
                        data_real = QString::number(data_temp2.toLongLong(&ok, 16));
                        data_before = 0;
                        data_temp2.clear();
                    }
                } else {
                    if (byte == 4) {
                        data_temp1.sprintf("%04x", data);
                        data_temp2.append(data_temp1);
                    } else if (byte == 2) {
                        data_before = data;
                    }
                }

                if (!data_real.isEmpty()) {
                    QStringList redis_config = cfg.read("REDIS");
                    rds.reqRedis("hset monita_service:" + monita_cfg.config.at(3) + dt_req_mod.date().toString("dd_MM_yyyy") + " " +
                                   QString::number(slave) + ";" +
                                   QString::number(addr+(i-1)) +
                                   "_" +
                                   dt_req_mod.toString("dd-MM-yyyy_HH:mm:ss:zzz") +
                                   " " +
                                   data_real, redis_config.at(0), redis_config.at(1).toInt());
                    logsheet = false;
                    for (int j = monita_cfg.config.at(2).toInt(); j > 0; j--) {
                        if (monita_cfg.modbus_period <= 1 * monita_cfg.jml_sumber ||
                                (monita_cfg.modbus_period > (monita_cfg.config.at(0).toInt()/j) * monita_cfg.jml_sumber &&
                                monita_cfg.modbus_period <= ((monita_cfg.config.at(0).toInt()/j)+1) * monita_cfg.jml_sumber)
                           ) {
                            rds.reqRedis("hset monita_service:temp " +
                                           QString::number(slave) + ";" +
                                           QString::number(addr+(i-1)) +
                                           "_" +
                                           dt_req_mod.toString("dd-MM-yyyy_HH:mm:ss:zzz") +
                                           " " +
                                           data_real, redis_config.at(0), redis_config.at(1).toInt());
//                            log.write("Debug","On Temp Redis");
                            logsheet = true;
                        }
                    }
                    rds.reqRedis("hset monita_service:vismon " +
                                   QString::number(slave) + ";" +
                                   QString::number(addr+(i-1)) +
                                   " " +
                                   data_real, redis_config.at(0), redis_config.at(1).toInt());
                    log.write("TcpModbus",
                              QString::number(slave) + " - " +
                              QString::number(addr+(i-1)) + " - " +
                              data_int + " - " +
                              data_hex + " - " +
                              data_real,
                              monita_cfg.config.at(7).toInt());
                }
                data_real.clear();
            }
        }
    }
    else
    {
        if( ret < 0 )
        {
            if(
#ifdef WIN32
                    errno == WSAETIMEDOUT ||
#endif
                    errno == EIO
                                                                    )
            {
                log.write("TcpModbus","I/O error : did not receive any data from slave ..",
                          monita_cfg.config.at(7).toInt());
                releaseTcpModbus();
            }
            else
            {
                log.write("TcpModbus","Protocol Error : Slave threw exception " +
                          QString::fromUtf8(modbus_strerror(errno)) +
                          " or function not implemented.",
                          monita_cfg.config.at(7).toInt());
                releaseTcpModbus();
            }
        }
        else
        {
            log.write("TcpModbus","Protocol Error : "
                                           "Number of registers returned does not match number of registers requested!",
                      monita_cfg.config.at(7).toInt());
            releaseTcpModbus();
        }
    }
}

int tcp_modbus::stringToHex(QString s)
{
    return s.replace( "0x", "" ).toInt( NULL, 16 );
}

QString tcp_modbus::embracedString(const QString s)
{
    return s.section( '(', 1 ).section( ')', 0, 0 );
}

QString tcp_modbus::descriptiveDataTypeName(int funcCode)
{
    switch( funcCode )
    {
        case MODBUS_FC_READ_COILS:
        case MODBUS_FC_WRITE_SINGLE_COIL:
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
            return "Coil (binary)";
        case MODBUS_FC_READ_DISCRETE_INPUTS:
            return "Discrete Input (binary)";
        case MODBUS_FC_READ_HOLDING_REGISTERS:
        case MODBUS_FC_WRITE_SINGLE_REGISTER:
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
            return "Holding Register (16 bit)";
        case MODBUS_FC_READ_INPUT_REGISTERS:
            return "Input Register (16 bit)";
        default:
            break;
    }
    return "Unknown";
}

void tcp_modbus::LuaRedis_function(QDateTime dt_lua)
{
    QStringList redis_config = cfg.read("REDIS");
    monita_cfg.funct_config = cfg.read("FUNCT");
    QStringList result; QString script_path;
    for (int i = 0; i < monita_cfg.funct_config.length(); i++) {
        if (QFileInfo(monita_cfg.funct_config.at(i)).exists() &&
                QFileInfo(monita_cfg.funct_config.at(i)).isFile()) {
            script_path = monita_cfg.funct_config.at(i);
            result = rds.eval(rds.readLua(script_path), redis_config.at(0), redis_config.at(1).toInt());
            if (result.length() < 2) {
                if (result.at(0).indexOf("Err") > 0) {
                    log.write("Lua", result.at(0),
                              monita_cfg.config.at(7).toInt());
//                    funct_temp.append(script_path + ":LuaError:" + result.at(0));
                } else {
                    funct_temp.append(result.at(0));
                }
            }
        }
    }

    this->send_ResultToRedis(funct_temp, dt_lua);
    funct_temp.clear();
}

void tcp_modbus::send_ResultToRedis(QStringList result_data, QDateTime dt_result)
{
    QStringList redis_config = cfg.read("REDIS");
    QStringList list_temp;
    for (int i = 0; i < result_data.length(); i++) {
        list_temp = result_data.at(i).split("*");
        if (list_temp.length() == 3 ) {
            rds.reqRedis("hset monita_service:" + monita_cfg.config.at(3) + dt_result.date().toString("dd_MM_yyyy") + " " +
                         list_temp.at(0) + ";" +
                         list_temp.at(1) +
                         "_" +
                         dt_result.toString("dd-MM-yyyy_HH:mm:ss:zzz") +
                         " " +
                         list_temp.at(2), redis_config.at(0), redis_config.at(1).toInt());
            rds.reqRedis("hset monita_service:vismon " +
                         list_temp.at(0) + ";" +
                         list_temp.at(1) +
                         " " +
                         list_temp.at(2), redis_config.at(0), redis_config.at(1).toInt());
            if (logsheet) {
                rds.reqRedis("hset monita_service:temp " +
                             list_temp.at(0) + ";" +
                             list_temp.at(1) +
                             "_" +
                             dt_result.toString("dd-MM-yyyy_HH:mm:ss:zzz") +
                             " " +
                             list_temp.at(2), redis_config.at(0), redis_config.at(1).toInt());
//                log.write("Debug","Result On Temp Redis");
            } else {
//                log.write("Debug","Result Not On Temp Redis");
            }
        } else {
//            rds.reqRedis("hset monita_service:vismon UNKNOWN" +
//                         result_data.at(i), redis_config.at(0), redis_config.at(1).toInt());
        }
    }
}

void tcp_modbus::releaseTcpModbus()
{
    if( m_tcpModbus ) {
        modbus_close( m_tcpModbus );
        modbus_free( m_tcpModbus );
        m_tcpModbus = NULL;
    }
}

void tcp_modbus::connectTcpModbus(const QString &address, int portNbr)
{
    releaseTcpModbus();

    m_tcpModbus = modbus_new_tcp( address.toLatin1().constData(), portNbr );
    if( modbus_connect( m_tcpModbus ) == -1 )
    {
        log.write("TcpModbus",address + ":" + QString::number(portNbr) + " Could not connect ..",
                  monita_cfg.config.at(7).toInt());
        releaseTcpModbus();
    } else {
        log.write("TcpModbus",address + ":" + QString::number(portNbr) + " Connected ..",
                  monita_cfg.config.at(7).toInt());
    }
}

void tcp_modbus::resetStatus()
{
    log.write("TcpModbus","Reset : Ready ..",
              monita_cfg.config.at(7).toInt());
}

void tcp_modbus::pollForDataOnBus()
{
    if(m_tcpModbus)
    {
        modbus_poll(m_tcpModbus);
    }
}

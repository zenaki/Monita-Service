#include "worker.h"

#include "modbus-tcp.h"

Worker::Worker(QObject *parent) : QObject(parent){
    fileName.sprintf("%s/log/log.txt", QDir::currentPath().toUtf8().data());

    QFile file(fileName);

    files = &file;
    files->open(QIODevice::WriteOnly);

    jml_sumber = 0;
    Modbus_Config.clear();
    list_config = cfg.read("source");
    if (list_config.length() > 7) {
        Modbus_Config = list_config;
        jml_sumber = list_config.length()/7;
    } else {
        Modbus_Config = list_config;
        jml_sumber++;
    }

//    period_cfg = cfg.read("config");

    connect(&timer, SIGNAL(timeout()), this, SLOT(doWork()));
//    timer.start((1000 * 60 * 10) / 2); /* 5 menit */
//    timer.start(period_cfg.at(1).toInt());
    timer.start(PERIODE);

    modbus_period = 0;
    this->doWork();
}

void Worker::doWork()
{
    timer.stop();
    for (int i = 0; i < jml_sumber; i++) {
        if (Modbus_Config.at(6) == "TCP") {
            this->connectTcpModbus(Modbus_Config.at(i*7), Modbus_Config.at(i*7+1).toInt());
            if (!m_tcpModbus) {
                Modbus_Config.clear();
                jml_sumber = 0;
                list_config = cfg.read("source");
                if (list_config.length() > 7) {
                    for (int i = 0; i < list_config.length()/7; i++) {
                        Modbus_Config = list_config;
                        jml_sumber++;
                    }
                } else {
                    Modbus_Config = list_config;
                    jml_sumber++;
                }
                this->connectTcpModbus(Modbus_Config.at(i*7), Modbus_Config.at(i*7+1).toInt());
                this->request(i);
                modbus_period++;
                if (modbus_period >= MODBUS_PERIOD*jml_sumber) {
                    QStringList redis_config = cfg.read("redis");
                    this->set_dataHarian(redis_config.at(0), redis_config.at(1).toInt());
                    modbus_period = 0;
                }
                releaseTcpModbus();
            } else {
                this->request(i);
                modbus_period++;
                if (modbus_period >= MODBUS_PERIOD*jml_sumber) {
                    QStringList redis_config = cfg.read("redis");
                    this->set_dataHarian(redis_config.at(0), redis_config.at(1).toInt());
                    modbus_period = 0;
                }
                releaseTcpModbus();
            }
        }
    }
    timer.start(PERIODE);
}

void Worker::set_dataHarian(QString address, int port)
{
    QStringList request = this->reqRedis("hlen data_jaman_" + QDate::currentDate().toString("dd_MM_yyyy"), address, port);
    int redis_len = request.at(0).toInt();
//    if (redis_len >= 30) {

//    }
    request = this->reqRedis("hgetall data_jaman_" + QDate::currentDate().toString("dd_MM_yyyy"), address, port, redis_len*2);
//    qSort(request.begin(), request.end());
    QStringList temp1; QStringList temp2; QString data;
    QStringList slave; QStringList id_titik_ukur;
    QStringList tanggal; QStringList data_tunggal; QStringList waktu;
    for (int i = 0; i < request.length(); i+=2) {
        temp1 = request.at(i).split("_");
        temp2 = temp1.at(0).split(";");
        slave.append(temp2.at(0));
        id_titik_ukur.append(temp2.at(1));
        tanggal.append(temp1.at(1).split("-"));
        waktu.append(temp1.at(2).split(":"));
        data_tunggal.append(request.at(i+1));
    }
    int t = 0;
    while (!db.isOpen()) {
        db = mysql.connect_db();
        db.open();
        if (!db.isOpen()) {
            qDebug() << "Monita::Database::Error : Connecting Fail ..!!";
            QThread::msleep(DELAY_DB_CONNECT);
            t++;
            if (t >= 3) {emit finish();}
        }
    }
    if (!get.check_table_is_available(db, "data_" + QDate::currentDate().toString("yyyy_MM_dd"))) {
        set.create_tabel_data_harian(db, QDate::currentDate().toString("yyyy_MM_dd"));
    }
    for (int i = 0; i < redis_len; i++) {
        data = data + "(" +
                id_titik_ukur.at(i) + ", " +
                slave.at(i) + ", " +
                tanggal.at(i*3+2) + tanggal.at(i*3+1) + tanggal.at(i*3) +
                waktu.at(i*4) + waktu.at(i*4+1) + waktu.at(i*4+2) + waktu.at(i*4+3) + ", " +
                data_tunggal.at(i) + ", " +
                waktu.at(i*4) + ", " +
                waktu.at(i*4+1) + ", " +
                waktu.at(i*4+2) +
        ")";
        if (i != redis_len - 1) {
            data = data + ",";
        }
        QThread::msleep(10);
    }
    while (!db.isOpen()) {
        db = mysql.connect_db();
        db.open();
        if (!db.isOpen()) {
            qDebug() << "Monita::Database::Error : Connecting Fail ..!!";
            QThread::msleep(DELAY_DB_CONNECT);
            t++;
            if (t >= 3) {emit finish();}
        }
    }
    set.data_harian(db, QDate::currentDate().toString("yyyy_MM_dd"), data);
    db.close();
//    qDebug() << "Monita::Database::Data Inserted on data_jaman_" + QDate::currentDate().toString("dd_MM_yyyy") + " ..";
    this->reqRedis("del data_jaman_" + QDate::currentDate().toString("dd_MM_yyyy"), address, port);
}

void Worker::releaseTcpModbus()
{
    if( m_tcpModbus ) {
        modbus_close( m_tcpModbus );
        modbus_free( m_tcpModbus );
        m_tcpModbus = NULL;
    }
}

void Worker::connectTcpModbus(const QString &address, int portNbr)
{
    releaseTcpModbus();

    m_tcpModbus = modbus_new_tcp( address.toLatin1().constData(), portNbr );
    if( modbus_connect( m_tcpModbus ) == -1 )
    {
//        printf("Monita::TcpModbus::Could not connecting ..\n");
        qDebug() << "Monita::TcpModbus::" + address + ":" + QString::number(portNbr) + " Could not connect ..";
        releaseTcpModbus();
    } else {
//        printf("Monita::TcpModbus::Connecting Success ..\n");
        qDebug() << "Monita::TcpModbus::" + address + ":" + QString::number(portNbr) + " Connected ..";
    }
}

void Worker::request(int index)
{
    if( m_tcpModbus == NULL )
    {
        return;
    }

    const int slave = Modbus_Config.at(index*7+2).toInt();
    const int func = Modbus_Config.at(index*7+3).toInt();
    const int addr = Modbus_Config.at(index*7+4).toInt();
    int num = Modbus_Config.at(index*7+5).toInt();

    uint8_t dest[1024];
    uint16_t *dest16 = (uint16_t *) dest;

    memset( dest, 0, 1024 );

    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString dataType = descriptiveDataTypeName(func);

    modbus_set_slave( m_tcpModbus, slave );
    qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy - HH:mm:ss:zzz") + " -------------------------------------------------------";
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
//            m_statusText->setText(
//                    tr( "Values successfully sent" ) );
//            m_statusInd->setStyleSheet( "background: #0b0;" );
            printf("Monita::TcpModbus::Value successfully sent ..\n");
            QTimer::singleShot( 2000, this, SLOT( resetStatus() ) );
        }
        else
        {
//            bool b_hex = is16Bit && ui->cbHexData->checkState() == Qt::Checked;
//            b_hex = true;
            QString qs_num;
            QString data_int;
            QString data_hex;
            int data_before;
            QByteArray array;
            QString data_real;

//            ui->regTable->setRowCount( num );
            for( int i = 0; i < num; ++i )
            {
                int data = is16Bit ? dest16[i] : dest[i];

//                QTableWidgetItem * dtItem = new QTableWidgetItem(dataType);
//                QTableWidgetItem * addrItem = new QTableWidgetItem(QString::number(addr+i));
//                qs_num.sprintf(b_hex ? "0x%04x" : "%d", data);
                data_int.sprintf("%d", data);
                data_hex.sprintf("0x%04x", data);
//                data_real.sprintf("%f", data);
                if (i % 2 == 1) {
                    data_real.sprintf("%04x%04x", data_before, data);
                    bool ok;
                    int sign = 1;
//                    QByteArray array("425AE78F");
                    array = data_real.toUtf8();
                    array = QByteArray::number(array.toLongLong(&ok,16),2); //convert hex to binary -you don't need this since your incoming data is binary
                    if(array.length()==32) {
                        if(array.at(0)=='1')  sign =-1;                       // if bit 0 is 1 number is negative
                        array.remove(0,1);                                     // remove sign bit
                    }
                    QByteArray fraction =array.right(23);   //get the fractional part
                    double mantissa = 0;
                    for(int i=0;i<fraction.length();i++)     // iterate through the array to claculate the fraction as a decimal.
                        if(fraction.at(i)=='1')     mantissa += 1.0/(pow(2,i+1));
                    int exponent = array.left(array.length()-23).toLongLong(&ok,2)-127;     //claculate the exponent
//                    qDebug() << "number= "<< QString::number( sign*pow(2,exponent)*(mantissa+1.0),'f', 5 );
//                    data_real.sprintf("%f", f_dt);
                    data_real = QString::number( sign*pow(2,exponent)*(mantissa+1.0),'f', 5 );
                } else {
                    data_before = data;
                }

//                QTableWidgetItem * dataItem = new QTableWidgetItem(data_int);
//                QTableWidgetItem * hexItem = new QTableWidgetItem(data_hex);
//                QTableWidgetItem * realItem = new QTableWidgetItem(data_real);
//                dtItem->setFlags( dtItem->flags() &
//                            ~Qt::ItemIsEditable );
//                addrItem->setFlags( addrItem->flags() &
//                            ~Qt::ItemIsEditable );
//                dataItem->setFlags( dataItem->flags() &
//                            ~Qt::ItemIsEditable );
//                hexItem->setFlags( hexItem->flags() &
//                            ~Qt::ItemIsEditable );
//                realItem->setFlags( realItem->flags() &
//                            ~Qt::ItemIsEditable );

//                ui->regTable->setItem(i,0,dtItem);
//                ui->regTable->setItem(i,1,addrItem);
//                ui->regTable->setItem(i,2,dataItem);
//                ui->regTable->setItem(i,3,hexItem);
//                ui->regTable->setItem(i,4,realItem);
                if (!data_real.isEmpty()) {
                    QStringList redis_config = cfg.read("redis");
                    this->reqRedis("hset data_jaman_" + QDate::currentDate().toString("dd_MM_yyyy") + " " +
                                   QString::number(slave) + ";" +
                                   QString::number(addr+i) +
                                   "_" +
                                   QDateTime::currentDateTime().toString("dd-MM-yyyy_HH:mm:ss:zzz") +
                                   " " +
                                   data_real, redis_config.at(0), redis_config.at(1).toInt());
                    qDebug() << "Monita::TcpModbus::" +
                                QString::number(slave) + " - " +
                                QString::number(addr+i) + " - " +
                                data_int + " - " +
                                data_hex + " - " +
                                data_real;
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
//                QMessageBox::critical( this, tr( "I/O error" ),
//                    tr( "I/O error: did not receive any data from slave." ) );
//                qDebug() << "I/O error::I/O error: did not receive any data from slave.";
                printf("Monita::TcpModbus::I/O error:: did not receive any data from slave ..\n");
                releaseTcpModbus();
            }
            else
            {
//                QMessageBox::critical( this, tr( "Protocol error" ),
//                    tr( "Slave threw exception \"%1\" or "
//                        "function not implemented." ).
//                                arg( modbus_strerror( errno ) ) );
                qDebug() << "Monita::TcpModbus::Protocol Error::Slave threw exception " +
                            QString::fromUtf8(modbus_strerror(errno)) +
                            " or function not implemented.";
                releaseTcpModbus();
            }
        }
        else
        {
//            QMessageBox::critical( this, tr( "Protocol error" ),
//                tr( "Number of registers returned does not "
//                    "match number of registers "
//                            "requested!" ) );
            qDebug() << "Monita::TcpModbus::Protocol Error::"
                        "Number of registers returned does not match number of registers requested!";
            releaseTcpModbus();
        }
    }
}

int Worker::stringToHex(QString s)
{
    return s.replace( "0x", "" ).toInt( NULL, 16 );
}

QString Worker::embracedString(const QString s)
{
    return s.section( '(', 1 ).section( ')', 0, 0 );
}

QString Worker::descriptiveDataTypeName(int funcCode)
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

QStringList Worker::reqRedis(QString command, QString address, int port, int len)
{
//    redisReply *reply;
//    redisContext *context = redisConnect(address.toStdString().c_str(), port);
//    QString command = "ping";
//    struct timeval timeout_context;
//    timeout_context.tv_sec = 0;
//    timeout_context.tv_usec = 500;

    QByteArray bytes = command.toLocal8Bit();
    QStringList result;
    r_context = redisConnect(address.toStdString().c_str(), port);
//    r_context = redisConnectWithTimeout(address.toStdString().c_str(), port, timeout_context);
    if (r_context != NULL && r_context->err)
    {
        printf("Monita::Redis::Error : %s\n", r_context->errstr);
    }

    void *pointer = NULL;
    pointer = redisCommand(r_context, bytes.constData());
    r_reply = (redisReply*)pointer;
    if ( r_reply->type == REDIS_REPLY_ERROR )
        printf( "Error: %s\n", r_reply->str );
    else if ( r_reply->type != REDIS_REPLY_ARRAY ) {
        if ( r_reply->type == REDIS_REPLY_INTEGER ) {
            result.insert(result.length(), QString::number(r_reply->integer));
//            qDebug() << "Monita::Redis::Result: " << QString::number(r_reply->integer);
        } else {
            result.insert(result.length(), r_reply->str);
//            qDebug() << "Monita::Redis::Result: " << QString(r_reply->str);
        }
    } else {
        for (int i = 0; i < len ; ++i ) {
            result.insert(result.length(), r_reply->element[i]->str);
            QThread::msleep(10);
//            printf( "Monita::Redis::Result:%i: %s\n", i,
//                r_reply->element[i]->str );
        }
    }
    freeReplyObject(r_reply);
    redisFree(r_context);
    return result;
}

void Worker::resetStatus( void )
{
//    m_statusText->setText( tr( "Ready" ) );
//    m_statusInd->setStyleSheet( "background: #aaa;" );
    qDebug() << "Monita::TcpModbus::Reset : Ready ..";
}

void Worker::pollForDataOnBus( void )
{
    if(m_tcpModbus)
    {
        modbus_poll(m_tcpModbus);
    }
}

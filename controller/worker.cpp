#include "worker.h"

#include "modbus-tcp.h"

Worker::Worker(QObject *parent) : QObject(parent)
{
    monita_cfg.jml_sumber = 0;
    monita_cfg.source_config = cfg.read("SOURCE");
    if (monita_cfg.source_config.length() > 7) {monita_cfg.jml_sumber = monita_cfg.source_config.length()/7;
    } else {monita_cfg.jml_sumber++;}

    manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply *)));

    monita_cfg.config = cfg.read("CONFIG");
    monita_cfg.calc_config = cfg.read("CALC");

    connect(&timer, SIGNAL(timeout()), this, SLOT(doWork()));
//    timer.start((1000 * 60 * 10) / 2); /* 5 menit */
    timer.start(monita_cfg.config.at(1).toInt());

    monita_cfg.modbus_period = 0;

    marine = (struct sky_wave_ship *) malloc( sizeof (struct sky_wave_ship));
    memset((char *) marine, 0, sizeof(struct sky_wave_ship));

    acc = (struct sky_wave_account *) malloc ( sizeof (struct sky_wave_account));
    memset((char *) acc, 0, sizeof(struct sky_wave_account));

//    get.modem_info(db, marine);
//    get.modem_getway(db, acc);

    ship_count = 0;
    gateway_count = 0;
    cnt_panggil = 0;
    calc_temp.clear();

    this->doWork();
}

void Worker::doWork()
{
//    timer.stop();
    QDateTime dateTime = QDateTime::currentDateTime();
    for (int i = 0; i < monita_cfg.jml_sumber; i++) {
        if (monita_cfg.source_config.at(6) == "TCP") {
            m_tcpModbus = NULL;
            this->connectTcpModbus(monita_cfg.source_config.at(i*7), monita_cfg.source_config.at(i*7+1).toInt());
            if (!m_tcpModbus) {
                monita_cfg.jml_sumber = 0;
                monita_cfg.source_config = cfg.read("SOURCE");
                if (monita_cfg.source_config.length() > 7) {monita_cfg.jml_sumber = monita_cfg.source_config.length()/7;
                } else {monita_cfg.jml_sumber++;}

                this->connectTcpModbus(monita_cfg.source_config.at(i*7), monita_cfg.source_config.at(i*7+1).toInt());
                if (m_tcpModbus) {
                    this->request_modbus(i, dateTime);
//                    monita_cfg.modbus_period++;
                    if (monita_cfg.modbus_period >= monita_cfg.config.at(0).toInt()*monita_cfg.jml_sumber) {
                        log.write(monita_cfg.config.at(3),"Debug",
                                  "Modbus_Periode : " + QString::number(monita_cfg.modbus_period) + "; " +
                                  "Modbus_Interval : " + monita_cfg.config.at(0) +"; " +
                                  "Jumlah_Sumber : " + QString::number(monita_cfg.jml_sumber));
                        monita_cfg.redis_config = cfg.read("REDIS");
                        this->set_dataHarian(monita_cfg.redis_config.at(0), monita_cfg.redis_config.at(1).toInt(), dateTime);
                        monita_cfg.modbus_period = 0;
                    }
                    releaseTcpModbus();
                }
                releaseTcpModbus();
            } else {
                this->request_modbus(i, dateTime);
//                monita_cfg.modbus_period++;
                if (monita_cfg.modbus_period >= monita_cfg.config.at(0).toInt()*monita_cfg.jml_sumber) {
                    log.write(monita_cfg.config.at(3),"Debug",
                              "Modbus_Periode : " + QString::number(monita_cfg.modbus_period) + "; " +
                              "Modbus_Interval : " + monita_cfg.config.at(0) +"; " +
                              "Jumlah_Sumber : " + QString::number(monita_cfg.jml_sumber));
                    QStringList redis_config = cfg.read("REDIS");
                    this->set_dataHarian(redis_config.at(0), redis_config.at(1).toInt(), dateTime);
                    monita_cfg.modbus_period = 0;
                }
                releaseTcpModbus();
            }
            monita_cfg.modbus_period++;
        } else if (monita_cfg.source_config.at(6) == "SKYW") {
//            this->request_sky_wave();
        }
    }
//    timer.start(monita_cfg.config.at(1).toInt());
}

void Worker::set_dataHarian(QString address, int port, QDateTime dt_sdh)
{
//    QStringList request = rds.reqRedis("hlen data_jaman_" + QDate::currentDate().toString("dd_MM_yyyy"), address, port);
    QStringList request = rds.reqRedis("hlen temp", address, port);
    log.write(monita_cfg.config.at(3),"Redis",request.at(0) + " Data ..");
    int redis_len = request.at(0).toInt();

//    request = rds.reqRedis("hgetall data_jaman_" + QDate::currentDate().toString("dd_MM_yyyy"), address, port, redis_len*2);
    request = rds.reqRedis("hgetall temp", address, port, redis_len*2);
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
        db.close();
        db = mysql.connect_db();
        db.open();
        if (!db.isOpen()) {
            log.write(monita_cfg.config.at(3),"Database","Error : Connecting Fail ..!!");
            QThread::msleep(DELAY_DB_CONNECT);
            t++;
            if (t >= 3) {emit finish();}
        }
    }
    if (!get.check_table_is_available(db, monita_cfg.config.at(5) + dt_sdh.date().toString("yyyy_MM_dd"))) {
        set.create_tabel_data_harian(db, monita_cfg.config.at(5), dt_sdh.date().toString("yyyy_MM_dd"));
    }
    for (int i = 0; i < redis_len; i++) {
//        data = data + "(" +
//                id_titik_ukur.at(i) + ", " +
//                slave.at(i) + ", " +
//                tanggal.at(i*3+2) + tanggal.at(i*3+1) + tanggal.at(i*3) +
//                waktu.at(i*4) + waktu.at(i*4+1) + waktu.at(i*4+2) + waktu.at(i*4+3) + ", " +
//                data_tunggal.at(i) + ", " +
//                waktu.at(i*4) + ", " +
//                waktu.at(i*4+1) + ", " +
//                waktu.at(i*4+2) +
//        ")";
        data = data + "(" +
                slave.at(i) + id_titik_ukur.at(i) + ", " +
                tanggal.at(i*3+2) + tanggal.at(i*3+1) + tanggal.at(i*3) +
                waktu.at(i*4) + waktu.at(i*4+1) + waktu.at(i*4+2) + waktu.at(i*4+3) + ", " +
                data_tunggal.at(i) + ", " +
                waktu.at(i*4) + ", " +
                waktu.at(i*4+1) + ", " +
                waktu.at(i*4+2) +
        ")";
//        log.write(monita_cfg.config.at(3),"Debug",id_titik_ukur.at(i));
        if (i != redis_len - 1) {
            data = data + ",";
        }
    }
    while (!db.isOpen()) {
        db.close();
        db = mysql.connect_db();
        db.open();
        if (!db.isOpen()) {
            log.write(monita_cfg.config.at(3),"Database","Error : Connecting Fail ..!!");
            QThread::msleep(DELAY_DB_CONNECT);
            t++;
            if (t >= 3) {emit finish();}
        }
    }
    set.data_harian(db, monita_cfg.config.at(5), dt_sdh.date().toString("yyyy_MM_dd"), data);
    db.close();
    log.write(monita_cfg.config.at(3),"Database","Data Inserted on table data_" + QDate::currentDate().toString("dd_MM_yyyy") + " ..");
//    rds.reqRedis("del data_jaman_" + QDate::currentDate().toString("dd_MM_yyyy"), address, port);
    rds.reqRedis("del temp", address, port);
    calc_temp.clear();
}

void Worker::request_sky_wave()
{
    QNetworkRequest request;

    monita_cfg.urls.sprintf("%s%s", acc->gway[monita_cfg.gateway_count].link, acc->gway[monita_cfg.gateway_count].nextutc);
    QUrl url =  QUrl::fromEncoded(monita_cfg.urls.toLocal8Bit().data());

    request.setUrl(url);
    manager->get(request);
}

void Worker::calculation(int slave_id, int reg, float data, bool logsheet, QDateTime dt_calc)
{
    QStringList list_temp1; QStringList list_temp2; QString temp;
    monita_cfg.calc_config = cfg.read("CALC");
    for (int k = 0; k < monita_cfg.calc_config.length(); k+=4) {
        list_temp1 = monita_cfg.calc_config.at(k+2).split(",");
        for (int l = 0; l < list_temp1.length(); l++) {
            list_temp2 = list_temp1.at(l).split(";");
            if (list_temp2.at(0) == QString::number(slave_id) && list_temp2.at(1) == QString::number(reg)) {
                if (monita_cfg.calc_config.at(k+1) == "SUM") {
                    if (list_temp2.length()>2) {if (list_temp2.at(2) == "ABS") {if (data < 0) {data = data * -1.0;}}}
                    temp = funct_sum(monita_cfg.calc_config.at(k).toInt(), monita_cfg.calc_config.at(k+3).toInt(), calc_temp, data);
                } else if (monita_cfg.calc_config.at(k+1) == "AVE") {
                    if (list_temp2.length()>2) {if (list_temp2.at(2) == "ABS") {if (data < 0) {data = data * -1.0;}}}
                    temp = funct_ave(monita_cfg.calc_config.at(k).toInt(), monita_cfg.calc_config.at(k+3).toInt(), calc_temp, data, list_temp1.length());
                } else if (monita_cfg.calc_config.at(k+1) == "MUL") {
                    if (list_temp2.length()>2) {if (list_temp2.at(2) == "ABS") {if (data < 0) {data = data * -1.0;}}}
                    temp = funct_mul(monita_cfg.calc_config.at(k).toInt(), monita_cfg.calc_config.at(k+3).toInt(), calc_temp, data);
                } else if (monita_cfg.calc_config.at(k+1) == "MIN") {
                    if (list_temp2.length()>2) {if (list_temp2.at(2) == "ABS") {if (data < 0) {data = data * -1.0;}}}
                    temp = funct_min(monita_cfg.calc_config.at(k).toInt(), monita_cfg.calc_config.at(k+3).toInt(), calc_temp, data);
                } else if (monita_cfg.calc_config.at(k+1) == "MAX") {
                    if (list_temp2.length()>2) {if (list_temp2.at(2) == "ABS") {if (data < 0) {data = data * -1.0;}}}
                    temp = funct_max(monita_cfg.calc_config.at(k).toInt(), monita_cfg.calc_config.at(k+3).toInt(), calc_temp, data);
                }
            }
        }
        if (!temp.isEmpty()) {
            QStringList redis_config = cfg.read("REDIS");
            list_temp1 = temp.split("*");
            rds.reqRedis("hset " + monita_cfg.config.at(4) + dt_calc.date().toString("dd_MM_yyyy") + " " +
                           list_temp1.at(0) + ";" +
                           list_temp1.at(1) +
                           "_" +
                           dt_calc.toString("dd-MM-yyyy_HH:mm:ss:zzz") +
                           " " +
                           list_temp1.at(2), redis_config.at(0), redis_config.at(1).toInt());
            if (logsheet) {
                rds.reqRedis("hset temp " +
                             list_temp1.at(0) + ";" +
                             list_temp1.at(1) +
                             "_" +
                             dt_calc.toString("dd-MM-yyyy_HH:mm:ss:zzz") +
                             " " +
                             list_temp1.at(2), redis_config.at(0), redis_config.at(1).toInt());
//                log.write(monita_cfg.config.at(3),"Debug","Calc On Temp Redis");
            } else {
//                log.write(monita_cfg.config.at(3),"Debug","Calc Not On Temp Redis");
            }
        }
    }
}

QString Worker::funct_sum(int id, int reg, QStringList calc_list, float data)
{
    QString temp; QStringList list_temp; QString data_temp; bool exc = false;
    if (calc_list.length() > 0) {
        for (int i = 0; i < calc_list.length(); i++) {
            list_temp = calc_list.at(i).split("*");
            if (list_temp.at(0) == QString::number(id) && list_temp.at(1) == QString::number(reg)) {
                data_temp = list_temp.at(2);
                temp = QString::number(id) + "*" + QString::number(reg) + "*" + QString::number(data_temp.toFloat() + data, 'f', 5);
                calc_list.replace(i, temp);
                exc = true;
                break;
            }
        }
        if (!exc) {
            temp = QString::number(id) + "*" + QString::number(reg) + "*" + QString::number(data, 'f', 5);
            calc_list.insert(calc_list.length(), temp);
        }
    } else {
        temp = QString::number(id) + "*" + QString::number(reg) + "*" + QString::number(data, 'f', 5);
        calc_list.insert(calc_list.length(), temp);
    }
    calc_temp = calc_list;
    return temp;
}

QString Worker::funct_ave(int id, int reg, QStringList calc_list, float data, int jml)
{
    QString temp; QStringList list_temp; QString data_temp; bool exc = false;
    if (calc_list.length() > 0) {
        for (int i = 0; i < calc_list.length(); i++) {
            list_temp = calc_list.at(i).split("*");
            if (list_temp.at(0) == QString::number(id) && list_temp.at(1) == QString::number(reg)) {
                data_temp = list_temp.at(2);
                temp = QString::number(id) + "*" + QString::number(reg) + "*" + QString::number(data_temp.toFloat() + (data / jml), 'f', 5);
                calc_list.replace(i, temp);
                exc = true;
                break;
            }
        }
        if (!exc) {
            temp = QString::number(id) + "*" + QString::number(reg) + "*" + QString::number(data, 'f', 5);
            calc_list.insert(calc_list.length(), temp);
        }
    } else {
        temp = QString::number(id) + "*" + QString::number(reg) + "*" + QString::number(data, 'f', 5);
        calc_list.insert(calc_list.length(), temp);
    }
    calc_temp = calc_list;
    return temp;
}

QString Worker::funct_mul(int id, int reg, QStringList calc_list, float data)
{
    QString temp; QStringList list_temp; QString data_temp; bool exc = false;
    if (calc_list.length() > 0) {
        for (int i = 0; i < calc_list.length(); i++) {
            list_temp = calc_list.at(i).split("*");
            if (list_temp.at(0) == QString::number(id) && list_temp.at(1) == QString::number(reg)) {
                data_temp = list_temp.at(2);
                temp = QString::number(id) + "*" + QString::number(reg) + "*" + QString::number(data_temp.toFloat() * data, 'f', 5);
                calc_list.replace(i, temp);
                exc = true;
                break;
            }
        }
        if (!exc) {
            temp = QString::number(id) + "*" + QString::number(reg) + "*" + QString::number(data, 'f', 5);
            calc_list.insert(calc_list.length(), temp);
        }
    } else {
        temp = QString::number(id) + "*" + QString::number(reg) + "*" + QString::number(data, 'f', 5);
        calc_list.insert(calc_list.length(), temp);
    }
    calc_temp = calc_list;
    return temp;
}

QString Worker::funct_min(int id, int reg, QStringList calc_list, float data)
{
    QString temp; QStringList list_temp; QString data_temp; bool exc = false;
    if (calc_list.length() > 0) {
        for (int i = 0; i < calc_list.length(); i++) {
            list_temp = calc_list.at(i).split("*");
            if (list_temp.at(0) == QString::number(id) && list_temp.at(1) == QString::number(reg)) {
                data_temp = list_temp.at(2);
                if (data_temp.toFloat() < data) {
                    temp = QString::number(id) + "*" + QString::number(reg) + "*" + data_temp;
                    calc_list.replace(i, temp);
                } else {
                    temp = QString::number(id) + "*" + QString::number(reg) + "*" + QString::number(data, 'f', 5);
                    calc_list.replace(i, temp);
                }
                exc = true;
                break;
            }
        }
        if (!exc) {
            temp = QString::number(id) + "*" + QString::number(reg) + "*" + QString::number(data, 'f', 5);
            calc_list.insert(calc_list.length(), temp);
        }
    } else {
        temp = QString::number(id) + "*" + QString::number(reg) + "*" + QString::number(data, 'f', 5);
        calc_list.insert(calc_list.length(), temp);
    }
    calc_temp = calc_list;
    return temp;
}

QString Worker::funct_max(int id, int reg, QStringList calc_list, float data)
{
    QString temp; QStringList list_temp; QString data_temp; bool exc = false;
    if (calc_list.length() > 0) {
        for (int i = 0; i < calc_list.length(); i++) {
            list_temp = calc_list.at(i).split("*");
            if (list_temp.at(0) == QString::number(id) && list_temp.at(1) == QString::number(reg)) {
                data_temp = list_temp.at(2);
                if (data_temp.toFloat() > data) {
                    temp = QString::number(id) + "*" + QString::number(reg) + "*" + data_temp;
                    calc_list.replace(i, temp);
                } else {
                    temp = QString::number(id) + "*" + QString::number(reg) + "*" + QString::number(data, 'f', 5);
                    calc_list.replace(i, temp);
                }
                exc = true;
                break;
            }
        }
        if (!exc) {
            temp = QString::number(id) + "*" + QString::number(reg) + "*" + QString::number(data, 'f', 5);
            calc_list.insert(calc_list.length(), temp);
        }
    } else {
        temp = QString::number(id) + "*" + QString::number(reg) + "*" + QString::number(data, 'f', 5);
        calc_list.insert(calc_list.length(), temp);
    }
    calc_temp = calc_list;
    return temp;
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
        log.write(monita_cfg.config.at(3),"TcpModbus",address + ":" + QString::number(portNbr) + " Could not connect ..");
        releaseTcpModbus();
    } else {
        log.write(monita_cfg.config.at(3),"TcpModbus",address + ":" + QString::number(portNbr) + " Connected ..");
    }
}

void Worker::request_modbus(int index, QDateTime dt_req_mod)
{
    if( m_tcpModbus == NULL )
    {
        return;
    }

    const int slave = monita_cfg.source_config.at(index*7+2).toInt();
    const int func = monita_cfg.source_config.at(index*7+3).toInt();
    const int addr = monita_cfg.source_config.at(index*7+4).toInt();
    int num = monita_cfg.source_config.at(index*7+5).toInt();

    uint8_t dest[1024];
    uint16_t *dest16 = (uint16_t *) dest;

    memset( dest, 0, 1024 );

    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString dataType = descriptiveDataTypeName(func);

    modbus_set_slave( m_tcpModbus, slave );
    log.write(monita_cfg.config.at(3),"TcpModbus", "Request from : " +
              monita_cfg.source_config.at(index*7) + ":" + monita_cfg.source_config.at(index*7+1));
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
            log.write(monita_cfg.config.at(3),"TcpModbus","Value successfully sent ..");
            QTimer::singleShot( 2000, this, SLOT( resetStatus() ) );
        }
        else
        {
            QString qs_num;
            QString data_int;
            QString data_hex;
            int data_before;
            QByteArray array;
            QString data_real;
            bool logsheet = false;

            for( int i = 0; i < num; ++i )
            {
                int data = is16Bit ? dest16[i] : dest[i];
                data_int.sprintf("%d", data);
                data_hex.sprintf("0x%04x", data);
                if (i % 2 == 1) {
                    data_real.sprintf("%04x%04x", data_before, data);
                    bool ok;
                    int sign = 1;
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
                    data_real = QString::number( sign*pow(2,exponent)*(mantissa+1.0),'f', 5 );
                } else {
                    data_before = data;
                }

                if (!data_real.isEmpty()) {
                    QStringList redis_config = cfg.read("REDIS");
                    rds.reqRedis("hset " + monita_cfg.config.at(4) + dt_req_mod.date().toString("dd_MM_yyyy") + " " +
                                   QString::number(slave) + ";" +
                                   QString::number(addr+i) +
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
                            rds.reqRedis("hset temp " +
                                           QString::number(slave) + ";" +
                                           QString::number(addr+i) +
                                           "_" +
                                           dt_req_mod.toString("dd-MM-yyyy_HH:mm:ss:zzz") +
                                           " " +
                                           data_real, redis_config.at(0), redis_config.at(1).toInt());
                            log.write(monita_cfg.config.at(3),"Debug","On Temp Redis");
                            logsheet = true;
                        }
                    }
                    log.write(monita_cfg.config.at(3),"TcpModbus",
                              QString::number(slave) + " - " +
                              QString::number(addr+i) + " - " +
                              data_int + " - " +
                              data_hex + " - " +
                              data_real);
                    this->calculation(slave, addr+i, data_real.toFloat(), logsheet, dt_req_mod);
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
                log.write(monita_cfg.config.at(3),"TcpModbus","I/O error : did not receive any data from slave ..");
                releaseTcpModbus();
            }
            else
            {
                log.write(monita_cfg.config.at(3),"TcpModbus","Protocol Error : Slave threw exception " +
                          QString::fromUtf8(modbus_strerror(errno)) +
                          " or function not implemented.");
                releaseTcpModbus();
            }
        }
        else
        {
            log.write(monita_cfg.config.at(3),"TcpModbus","Protocol Error : "
                                           "Number of registers returned does not match number of registers requested!");
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

void Worker::resetStatus( void )
{
    log.write(monita_cfg.config.at(3),"TcpModbus","Reset : Ready ..");
}

void Worker::pollForDataOnBus( void )
{
    if(m_tcpModbus)
    {
        modbus_poll(m_tcpModbus);
    }
}

void Worker::replyFinished(QNetworkReply* reply){
    QString xmlStr;
    xmlStr.clear();

    xmlStr=reply->readAll();
    read.parse_xml_account_methode(xmlStr, db, marine, acc, acc->gway[monita_cfg.gateway_count].id, monita_cfg.gateway_count);

    monita_cfg.gateway_count++;

    if (monita_cfg.gateway_count < acc->sum_getway){
        this->request_sky_wave();
    }
    else{
        monita_cfg.gateway_count = 0;
//        timer.start((1000 * 60 * 10) / 2);
    }
}

#include "sky_wave.h"

#define BYTE_TO_BINARY(byte) \
    (byte & 0x80 ? '1' : '0'), \
    (byte & 0x40 ? '1' : '0'), \
    (byte & 0x20 ? '1' : '0'), \
    (byte & 0x10 ? '1' : '0'), \
    (byte & 0x08 ? '1' : '0'), \
    (byte & 0x04 ? '1' : '0'), \
    (byte & 0x02 ? '1' : '0'), \
    (byte & 0x01 ? '1' : '0')

sky_wave::sky_wave(QObject *parent) : QObject(parent)
{
}

void sky_wave::doSetup(QThread &cThread)
{
    connect(&cThread,SIGNAL(started()),this,SLOT(doWork()));

    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply *)));

    monita_cfg.config = cfg.read("CONFIG");
    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(doWork()));
    t->start(monita_cfg.config.at(6).toInt() * 60000);

    marine = (struct sky_wave_ship *) malloc( sizeof (struct sky_wave_ship));
    memset((char *) marine, 0, sizeof(struct sky_wave_ship));

    acc = (struct sky_wave_account *) malloc ( sizeof (struct sky_wave_account));
    memset((char *) acc, 0, sizeof(struct sky_wave_account));

//    mysql.close(db_skywave);
    db_skywave = mysql.connect_db();
    get.modem_info(db_skywave, marine);
    get.modem_getway(db_skywave, acc);
    get.skyWave_config(db_skywave, &monita_cfg);

    ship_count = 0;
    gateway_count = 0;
    cnt_panggil = 0;
    
//    this->doWork();
}

void sky_wave::parsing(QByteArray data_json, int indexGateway)
{
    monita_cfg.config = cfg.read("CONFIG");
    bool ok;
    QString MessageUTC;
    QString RawBiner;
    QStringList PayLoadData;
    bool getData = false;
    int index;
    char temp[16];
    JsonDoc = QJsonDocument::fromJson(data_json);
    QJsonObject object = JsonDoc.object();
    MessageUTC = object.value("NextStartUTC").toString();
    monita_cfg.sky[indexGateway].next_utc = QDateTime::fromString(MessageUTC, "yyyy-MM-dd HH:mm:ss");

    QJsonArray array = object.value("Messages").toArray();
    if (!MessageUTC.isEmpty()) {
        log.write("SkyWave","Mulai Parsing URL " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz"), monita_cfg.config.at(7).toInt());
        foreach (const QJsonValue & v, array) {
            for (int i = 0; i < monita_cfg.sky[indexGateway].jml_modem; i++) {
                if (v.toObject().value("MobileID").toString() ==
                        monita_cfg.sky[indexGateway].mdm[i].modem_id) {
                    QStringList SIN_MIN = monita_cfg.sky[indexGateway].SIN_MIN.split(","); QStringList SIN_MIN_List;
                    for (int j = 0; j < SIN_MIN.length(); j++) {SIN_MIN_List.append(SIN_MIN.at(j).split(";"));}
                    for (int j = 0; j < SIN_MIN_List.length(); j+=2) {
                        if (v.toObject().value("SIN").toInt() == SIN_MIN_List.at(j).toInt()) {
                            monita_cfg.sky[indexGateway].mdm[i].val_tu.clear();
                            QJsonArray LoadArray  = v.toObject().value("RawPayload").toArray();
                            if (!LoadArray.isEmpty()) {
                                if (LoadArray.at(1).toInt() == SIN_MIN_List.at(j+1).toInt()) {
                                    index = 0;
                                    foreach (const QJsonValue & vRaw, LoadArray) {
                                        if (index > 1) {
                                            sprintf(temp, "%c%c%c%c%c%c%c%c", BYTE_TO_BINARY(vRaw.toInt()));
                                            RawBiner.append(temp);
                                        }
                                        index++;
                                    }
                                    PayLoadData = parsingRawPayload(parse.format_5cut_32get(RawBiner));
                                    for (int j = 5; j < PayLoadData.length(); j++) {
                                        if (PayLoadData.at(j) != "0.000000000") {
                                            getData = true;
                                            break;
                                        }
                                    }
            //                        listData.append("RawPayload:" + parse.format_5cut_32get(RawBiner));
                                    if (getData) {
                                        monita_cfg.sky[indexGateway].mdm[i].last_utc = PayLoadData.at(0);
                                        PayLoadData.removeAt(0);
                                        monita_cfg.sky[indexGateway].mdm[i].val_tu = PayLoadData;

                                        for (int k = 0; k < monita_cfg.sky[indexGateway].mdm[i].val_tu.length(); k++) {
                                            QString unixTimeStr = QString::number(monita_cfg.sky[indexGateway].mdm[i].last_utc.toInt());
                                            const uint s = unixTimeStr.toUInt( &ok );
                                            const QDateTime dt = QDateTime::fromTime_t( s );
                                            const QString date_gmt7 = dt.toString("yyyy-MM-dd HH:mm:ss");

                                            if (!monita_cfg.sky[indexGateway].mdm[i].query.isEmpty()) {
                                                monita_cfg.sky[indexGateway].mdm[i].query.sprintf(
                                                            "%s,(%d, %.9f, %d, %d, '%s', %d)",
                                                            monita_cfg.sky[indexGateway].mdm[i].query.toUtf8().data(),
                                                            monita_cfg.sky[indexGateway].mdm[i].id_tu.at(k).toInt(),
                                                            monita_cfg.sky[indexGateway].mdm[i].val_tu.at(k).toFloat(),
                                                            0,
                                                            monita_cfg.sky[indexGateway].mdm[i].last_utc.toInt(),
                                                            date_gmt7.toUtf8().data(),
                                                            FLAG_ENGINE_DATA);
                                            } else {
                                                monita_cfg.sky[indexGateway].mdm[i].query.sprintf(
                                                            "(%d, %.9f, %d, %d, '%s', %d)",
                                                            monita_cfg.sky[indexGateway].mdm[i].id_tu.at(k).toInt(),
                                                            monita_cfg.sky[indexGateway].mdm[i].val_tu.at(k).toFloat(),
                                                            0,
                                                            monita_cfg.sky[indexGateway].mdm[i].last_utc.toInt(),
                                                            date_gmt7.toUtf8().data(),
                                                            FLAG_ENGINE_DATA);
                                            }
                                        }
                                    }
    //                                qDebug() << monita_cfg.sky[indexGateway].mdm[i].query;
                                    RawBiner.clear();
                                }
                            }
                            if ((v.toObject().value("Payload").toObject().value("SIN").toInt() == SIN_MIN_List.at(j).toInt()) &&
                                    (v.toObject().value("Payload").toObject().value("MIN").toInt() == SIN_MIN_List.at(j+1).toInt())) {
                                if (v.toObject().value("Payload").toObject().value("Name").toString() == DATA_TRACKING) {
                                    monita_cfg.sky[indexGateway].mdm[i].val_tu.clear();
                                    QJsonArray Field_Payload = v.toObject().value("Payload").toObject().value("Fields").toArray();
                                    for (int j = 0; j < 4; j++) {monita_cfg.sky[indexGateway].mdm[i].val_tu.append("N/A");}
                                    foreach (const QJsonValue vField, Field_Payload) {
                                        if (vField.toObject().value("Name").toString() == "fixTime") {
                                            monita_cfg.sky[indexGateway].mdm[i].last_utc = vField.toObject().value("Value").toString();
                                        } else if (vField.toObject().value("Name").toString() == "latitude") {
                                            monita_cfg.sky[indexGateway].mdm[i].val_tu[0] = vField.toObject().value("Value").toString();
                                            monita_cfg.sky[indexGateway].mdm[i].val_tu[0] = QString::number(monita_cfg.sky[indexGateway].mdm[i].val_tu.at(0).toFloat()/60000.000000000, 'f', 9);
                                        } else if (vField.toObject().value("Name").toString() == "longitude") {
                                            monita_cfg.sky[indexGateway].mdm[i].val_tu[1] = vField.toObject().value("Value").toString();
                                            monita_cfg.sky[indexGateway].mdm[i].val_tu[1] = QString::number(monita_cfg.sky[indexGateway].mdm[i].val_tu.at(1).toFloat()/60000.000000000, 'f', 9);
                                        } else if (vField.toObject().value("Name").toString() == "heading") {
                                            monita_cfg.sky[indexGateway].mdm[i].val_tu[2] = vField.toObject().value("Value").toString();
                                        } else if (vField.toObject().value("Name").toString() == "speed") {
                                            monita_cfg.sky[indexGateway].mdm[i].val_tu[3] = vField.toObject().value("Value").toString();
                                        }
                                    }
                                    for (int k = 0; k < monita_cfg.sky[indexGateway].mdm[i].val_tu.length(); k++) {
                                        QString unixTimeStr = QString::number(monita_cfg.sky[indexGateway].mdm[i].last_utc.toInt());
                                        const uint s = unixTimeStr.toUInt( &ok );
                                        const QDateTime dt = QDateTime::fromTime_t( s );
                                        const QString date_gmt7 = dt.toString("yyyy-MM-dd HH:mm:ss");

                                        if (!monita_cfg.sky[indexGateway].mdm[i].query.isEmpty()) {
                                            monita_cfg.sky[indexGateway].mdm[i].query.sprintf(
                                                        "%s,(%d, %.9f, %d, %d, '%s', %d)",
                                                        monita_cfg.sky[indexGateway].mdm[i].query.toUtf8().data(),
                                                        monita_cfg.sky[indexGateway].mdm[i].id_tu.at(k).toInt(),
                                                        monita_cfg.sky[indexGateway].mdm[i].val_tu.at(k).toFloat(),
                                                        0,
                                                        monita_cfg.sky[indexGateway].mdm[i].last_utc.toInt(),
                                                        date_gmt7.toUtf8().data(),
                                                        FLAG_ENGINE_DATA);
                                        } else {
                                            monita_cfg.sky[indexGateway].mdm[i].query.sprintf(
                                                        "(%d, %.9f, %d, %d, '%s', %d)",
                                                        monita_cfg.sky[indexGateway].mdm[i].id_tu.at(k).toInt(),
                                                        monita_cfg.sky[indexGateway].mdm[i].val_tu.at(k).toFloat(),
                                                        0,
                                                        monita_cfg.sky[indexGateway].mdm[i].last_utc.toInt(),
                                                        date_gmt7.toUtf8().data(),
                                                        FLAG_ENGINE_DATA);
                                        }
                                    }
    //                                qDebug() << monita_cfg.sky[indexGateway].mdm[i].query;
                                }
                            }
    //                        if (!LoadArray.isEmpty()) {
    //                            listData.append("Payload:");
    //                        }
                        }
                    }
                }
            }
        }
        log.write("SkyWave","Mulai Proses ke Database " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz"), monita_cfg.config.at(7).toInt());
        while (!db_skywave.isOpen()) {
//            db_skywave.close();
            db_skywave.open();
            if (!db_skywave.isOpen()) {
                log.write("Database","Error : Connecting Fail ..!!",
                          monita_cfg.config.at(7).toInt());
                QThread::msleep(DELAY_DB_CONNECT);
                db_skywave = mysql.connect_db();
            }
        }
        if (!get.check_table_is_available(db_skywave, "data")) {
            set.create_table_data_punya_skywave(db_skywave, "data");
        }
        if (!get.check_table_is_available(db_skywave, "data_" + QDateTime::currentDateTime().toString("yyyyMMdd"))) {
            set.create_table_data_punya_skywave(db_skywave, "data_" + QDateTime::currentDateTime().toString("yyyyMMdd"));
        }
        QString dataToTable, vTarget, vIDTarget;
        for (int i = 0; i < monita_cfg.sky[indexGateway].jml_modem; i++) {
            if (!monita_cfg.sky[indexGateway].mdm[i].query.isEmpty()) {
                dataToTable = dataToTable + monita_cfg.sky[indexGateway].mdm[i].query;
                if (!monita_cfg.sky[indexGateway].mdm[i].last_utc.isEmpty()) {
                    QString unixTimeStr = monita_cfg.sky[indexGateway].mdm[i].last_utc;
                    const uint s = unixTimeStr.toUInt( &ok );
                    const QDateTime dt = QDateTime::fromTime_t( s );
                    const QString result = dt.toString("yyyy-MM-dd HH:mm:ss");
                    vTarget = vTarget +
                            "when id_ship = " + QString::number(monita_cfg.sky[indexGateway].mdm[i].id_ship) +
                            " then '" + result + "' ";
                    if (vIDTarget.isEmpty()) {
                        vIDTarget = QString::number(monita_cfg.sky[indexGateway].mdm[i].id_ship);
                    } else {
                        vIDTarget = vIDTarget + "," + QString::number(monita_cfg.sky[indexGateway].mdm[i].id_ship);
                    }
                }
                if (i < monita_cfg.sky[indexGateway].jml_modem-1) {
                    dataToTable = dataToTable + ",";
                }
            }
        }
        if (dataToTable.at(dataToTable.length()-1) == ',') {dataToTable.remove(dataToTable.length()-1, 1);}
    //    qDebug() << dataToTable;
        if (!dataToTable.isEmpty()) {
            set.data_punya_skywave(db_skywave, "data", dataToTable);
            set.data_punya_skywave(db_skywave, "daata_" + QDateTime::currentDateTime().toString("yyyyMMdd"), dataToTable);
            if (!vTarget.isEmpty()) {
                set.update_multiple_punya_skywave(db_skywave, "ship", "nextutc", "id_ship", vIDTarget, vTarget);
            }
            set.update_multiple_punya_skywave(db_skywave, "gateway", "next_utc", "id", QString::number(monita_cfg.sky[indexGateway].id_gateWay),
                                              "when id = " + QString::number(monita_cfg.sky[indexGateway].id_gateWay) + " then '" + MessageUTC + "' ");
        }
//        mysql.close(db_skywave);
        db_skywave.close();
        log.write("SkyWave","Selesai .. " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz"), monita_cfg.config.at(7).toInt());
    } else {
        log.write("SkyWave","Tidak Dapat Data " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz"), monita_cfg.config.at(7).toInt());
    }
}

QStringList sky_wave::parsingRawPayload(QString RawData)
{
    bool ok;
    int cnt_p = 0; int cnt_d = 0;
    QString data; QByteArray dataBin;
    QString data_decimal; QString data_real;
//    float data_f;
//    int epochtime;
//    QString dat_time;
    QStringList DataSkyWave;
//    QByteArray hexData;
    data.clear();
//    QStringList redis_config = cfg.read("REDIS");

//    parse.parse_data(&q, f_5c32g, marine->kapal[n].id_ship, f_mUTC, FLAG_ENGINE_DATA);
    char dats[RawData.size()+1];
    strcpy(dats, RawData.toLatin1());
    for (int j = 0; j < RawData.size(); j++) {
        cnt_p++;
        data.sprintf("%s%c", data.toLocal8Bit().data(), dats[j]);
        if (cnt_p == 32) {
            cnt_d++;
            int sign = 1;
            dataBin = data.toUtf8();
            if(dataBin.length()==32) {
                if(dataBin.at(0)=='1')  sign =-1;                       // if bit 0 is 1 number is negative
                dataBin.remove(0,1);                                     // remove sign bit
            }
            QByteArray fraction =dataBin.right(23);   //get the fractional part
            double mantissa = 0;
            for(int i=0;i<fraction.length();i++)     // iterate through the array to claculate the fraction as a decimal.
                if(fraction.at(i)=='1')     mantissa += 1.0/(pow(2,i+1));
            int exponent = dataBin.left(dataBin.length()-23).toLongLong(&ok,2)-127;     //claculate the exponent
            data_real = QString::number( sign*pow(2,exponent)*(mantissa+1.0),'f', 9 );
            data_decimal = QString::number( sign*pow(2,exponent)*(mantissa+1.0),'f', 0 );
            if (cnt_d == 1) {
//                QString unixTimeStr = data_decimal;
//                const uint s = unixTimeStr.toUInt( &ok );
//                const QDateTime dt = QDateTime::fromTime_t( s );
//                const QString result_textdate = dt.toString( Qt::TextDate );

                DataSkyWave.append(data_decimal);
//                DataSkyWave.append(result_textdate);
//                rds.reqRedis("hset monita_service:vismon " +
//                               QString::number(slave) + ";" +
//                               QString::number(addr+(i-1)) +
//                               " " +
//                               data_real, redis_config.at(0), redis_config.at(1).toInt());
            } else {
                DataSkyWave.append(data_real);
            }
            data.clear();
            cnt_p = 0;
        }
    }
//    log.write("SkyWave",
//              QString::number(slave) + " - " +
//              QString::number(addr+(i-1)) + " - " +
//              data_int + " - " +
//              data_hex + " - " +
//              data_real);
//    DataSkyWave.clear();
    return DataSkyWave;
}

void sky_wave::doWork()
{
    QNetworkRequest request;

    monita_cfg.urls.sprintf("%s%s", acc->gway[monita_cfg.gateway_count].link, acc->gway[monita_cfg.gateway_count].nextutc);
    monita_cfg.urls = monita_cfg.sky[monita_cfg.gateway_count].url + monita_cfg.sky[monita_cfg.gateway_count].next_utc.toString("yyyy-MM-dd%20HH:mm:ss");
//    qDebug() << monita_cfg.urls;
//    monita_cfg.urls = "http://m2prime.aissat.com/RestMessages.svc/get_return_messages.json/?access_id=150103286&password=ZRM3B9SSDI&start_utc=2016-09-02%2005:40:05";
//    qDebug() << monita_cfg.urls;
    QUrl url =  QUrl::fromEncoded(monita_cfg.urls.toLocal8Bit().data());

    request.setUrl(url);
    log.write("SkyWave","Send Request to URL " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz"), monita_cfg.config.at(7).toInt());
    manager->get(request);
}

void sky_wave::replyFinished(QNetworkReply* reply)
{
    log.write("SkyWave","Get Respone from SkyWave " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz"), monita_cfg.config.at(7).toInt());
    QByteArray data;
    data.clear();

    data = reply->readAll();
//    read.parse_xml_account_methode(xmlStr, db, marine, acc, acc->gway[monita_cfg.gateway_count].id, monita_cfg.gateway_count);
    this->parsing(data, monita_cfg.gateway_count);

    monita_cfg.gateway_count++;

    if (monita_cfg.gateway_count < monita_cfg.jml_gateWay) {
        this->doWork();
    } else {
        monita_cfg.gateway_count = 0;
//        timer.start((1000 * 60 * 10) / 2);
    }
}

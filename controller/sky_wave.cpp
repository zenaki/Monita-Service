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

    manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply *)));

    monita_cfg.sky_config = cfg.read("SKYWAVE");
    monita_cfg.config = cfg.read("CONFIG");

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(doWork()));
    t->start(monita_cfg.sky_config.at(0).toInt() * 60000);

    marine = (struct sky_wave_ship *) malloc( sizeof (struct sky_wave_ship));
    memset((char *) marine, 0, sizeof(struct sky_wave_ship));

    acc = (struct sky_wave_account *) malloc ( sizeof (struct sky_wave_account));
    memset((char *) acc, 0, sizeof(struct sky_wave_account));

    db = mysql.connect_db();
    get.modem_info(db, marine);
    get.modem_getway(db, acc);
    get.skyWave_config(db, &monita_cfg);

    ship_count = 0;
    gateway_count = 0;
    cnt_panggil = 0;
    
    this->doWork();
}

void sky_wave::parsing(QByteArray data_json)
{
    QString MessageUTC;
    QString RawBiner;
    QStringList listData;
    int index;
    char temp[16];
    JsonDoc = QJsonDocument::fromJson(data_json);
    QJsonObject object = JsonDoc.object();
    MessageUTC = object.value("NextStartUTC").toString();

    QJsonArray array = object.value("Messages").toArray();
    foreach (const QJsonValue & v, array) {
        if (v.toObject().value("SIN").toInt() == 128 || v.toObject().value("SIN").toInt() == 19) {
            QJsonArray LoadArray  = v.toObject().value("RawPayload").toArray();
            if (!LoadArray.isEmpty()) {
                index = 0;
                foreach (const QJsonValue & vRaw, LoadArray) {
                    if (index > 1) {
                        sprintf(temp, "%c%c%c%c%c%c%c%c", BYTE_TO_BINARY(vRaw.toInt()));
                        RawBiner.append(temp);
                    }
                    index++;
                }
                listData.append("RawPayload:" + parse.format_5cut_32get(RawBiner));
                RawBiner.clear();
            }
            LoadArray = v.toObject().value("Payload").toArray();
            if (!LoadArray.isEmpty()) {
                listData.append("Payload:");
            }
        }
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
    QStringList redis_config = cfg.read("REDIS");

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
            data_real = QString::number( sign*pow(2,exponent)*(mantissa+1.0),'f', 5 );
            data_decimal = QString::number( sign*pow(2,exponent)*(mantissa+1.0),'f', 0 );
            if (cnt_d == 1) {
                QString unixTimeStr = data_decimal;
                const uint s = unixTimeStr.toUInt( &ok );
                const QDateTime dt = QDateTime::fromTime_t( s );
                const QString result_textdate = dt.toString( Qt::TextDate );

                DataSkyWave.append(result_textdate);
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
//    monita_cfg.urls.sprintf("%s%s", monita_cfg.sky)
    monita_cfg.urls = "http://m2prime.aissat.com/RestMessages.svc/get_return_messages.json/?access_id=150103286&password=ZRM3B9SSDI&start_utc=2016-09-02%2005:40:05";
    QUrl url =  QUrl::fromEncoded(monita_cfg.urls.toLocal8Bit().data());

    request.setUrl(url);
    manager->get(request);
}

void sky_wave::replyFinished(QNetworkReply* reply){
    QByteArray data;
    data.clear();

    data = reply->readAll();
//    read.parse_xml_account_methode(xmlStr, db, marine, acc, acc->gway[monita_cfg.gateway_count].id, monita_cfg.gateway_count);
    this->parsing(data);

    monita_cfg.gateway_count++;

    if (monita_cfg.gateway_count < acc->sum_getway){
        this->doWork();
    }
    else{
        monita_cfg.gateway_count = 0;
//        timer.start((1000 * 60 * 10) / 2);
    }
}

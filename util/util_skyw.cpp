#include "util_skyw.h"

util_skyw::util_skyw(QObject *parent) :
    QObject(parent)
{
    buf_lay = (char *) malloc(1024);
    buf_dateTime = (char *) malloc(32);
}

void util_skyw::write(QFile *file, const char *text, ...){

    va_list args;
    int i;

    va_start (args, text);

       /*
        * For this to work, printbuffer must be larger than
        * anything we ever want to print.
        */

    i = vsprintf (buf_lay, text, args);
    va_end (args);

    /* Print the string */
    sprintf (buf_dateTime, " [%s]\r\n", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toUtf8().data(), args);

    strcat(buf_lay, buf_dateTime);

    QTextStream printLog(file);
    printLog << buf_lay;
}

#if 0
void util_skyw::parse_xml(QString skyw, QSqlQuery *q, int id_ship, int SIN, int MIN, struct utama *marine, int urut){

    printf("\nChecking for => id : %d --> SIN : %d --> modem id : %s --> kapal :%s\n",
           id_ship, SIN, marine->kapal[urut].modem_id, marine->kapal[urut].name);

    int cnt = 0;
    int cnt_tu = 1;

    int epochtime;
    QString dat_time;

    QXmlStreamReader xml;

    QString MessageUTC;
    int sin_xml;
    QString MobileID;
    QString RawPayload;

    QString cek_mobile;

    QString field_name;
    int value;

    QString decode;
    QString bin;
    QString f_5c32g;

    xml.clear();
    xml.addData(skyw);

    while(!xml.atEnd() &&  !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if(token == QXmlStreamReader::StartElement) {
            if (xml.name() == "MessageUTC"){
                MessageUTC.sprintf("%s", xml.readElementText().toUtf8().data());
            }
            if (xml.name() == "SIN"){
                sin_xml = xml.readElementText().toInt();
            }
            if (xml.name() == "MobileID"){
                MobileID.sprintf("%s", xml.readElementText().toUtf8().data());
            }

            /* validasi 0 --> match | -1 --> not match */
            if(strcmp(MobileID.toUtf8().data(), marine->kapal[urut].modem_id) == 0  && (sin_xml == marine->kapal[urut].SIN)){
                /* simpan data masuk */
                save.update_next_utc(q, MessageUTC, id_ship);

                /* SkyWave - Imani Prima */
                if (xml.name() == "RawPayload"){
                    printf("\nDate Time : %s (+07:00)\n", MessageUTC.toUtf8().data());
                    RawPayload.sprintf("%s", xml.readElementText().toUtf8().data());

                    decode.clear();
                    decode = parse.decode_base64(RawPayload);

                    bin.clear();
                    bin = parse.hex_to_bin_conversion(decode);

                    f_5c32g.clear();
                    f_5c32g = parse.format_5cut_32get(bin);

                    parse.parse_data(q, f_5c32g, id_ship);
                }

                /* SkyWave - KurayGeo */
                if (xml.name() == "Payload"){
                    printf("\nDate Time : %s (+07:00)\n", MessageUTC.toUtf8().data());
                    cnt = 0;
                    cnt_tu = 1;
                }
                if (xml.name() == "Field"){
                    QXmlStreamAttributes attributes = xml.attributes();
                    field_name = attributes.value("Name").toString();
                    value = attributes.value("Value").toString().toInt();

                    float data_f = *(float *) &value;

                    if (cnt == 0){
                        epochtime = (int) data_f;

                        const QDateTime time = QDateTime::fromTime_t((int)data_f);
                        dat_time = time.toString("yyyy-MM-dd hh:mm:ss").toLocal8Bit().data();

                        cnt = 1;
                    }
                    else{
                        q->clear();
                        int id_tu = get.id_tu_ship(q, id_ship, cnt_tu);
                        if (id_tu != 0){
                            q->clear();
                            printf("%d --> %.2f\n", id_tu, data_f);
                            save.data(q, data_f, id_tu, 0, epochtime, dat_time);
                        }
                        else{
                            printf("\nbelum di set parsing refnya");
                        }
                        cnt_tu++;
                    }
                }
            }
        }
    }
}
#endif

void util_skyw::parse_xml_account_methode(QString skyw, QSqlDatabase db, utama *marine, account *acc, int id_gateway, int index_gway){
#if 1
    if(id_gateway == MODEM_KURAYGEO){
        parse_kureyGeo(skyw, db, marine, acc, id_gateway, index_gway);
    }
    else if(id_gateway == MODEM_IMANIPRIMA){
        parse_imaniPrima(skyw, db, marine, acc, id_gateway, index_gway);
    }
#endif
}

void util_skyw::parse_kureyGeo(QString skyw, QSqlDatabase db, utama *marine, account *acc, int id_gateway, int index_gway){
#if 0
    int cnt = 0;
    int cnt_tu = 1;
    int n;

    int SIN;

    int epochtime;
    QString dat_time;

    QString MobileID;
    QString MessageUTC;

    QXmlStreamReader xml;

    xml.clear();
    xml.addData(skyw);

    while(!xml.atEnd() &&  !xml.hasError()){
        QXmlStreamReader::TokenType token = xml.readNext();
        if(token == QXmlStreamReader::StartElement){
            bool id_match = false;

            if (xml.name() == "MessageUTC"){
                MessageUTC.sprintf("%s", xml.readElementText().toUtf8().data());
                save.update_next_utc_gateway(q, MessageUTC, id_gateway);

                strcpy(acc->gway[id_gateway-1].nextutc, MessageUTC.toLatin1());
            }

            if (xml.name() == "SIN"){
                SIN = xml.readElementText().toInt();
            }

            if (xml.name() == "MobileID"){
                MobileID.sprintf("%s", xml.readElementText().toUtf8().data());
            }

            /* Filtering Mobile ID */
            for(int i = 0; i < marine->sum_ship; i++){
                if(marine->kapal[i].modem_id == MobileID){
                    n = i;
                    id_match = true;
                }
            }

            if(id_match){
                //save.update_next_utc(q, MessageUTC, marine->kapal[n].id_ship);

                if (xml.name() == "Payload"){
                    QXmlStreamAttributes attributes = xml.attributes();

                    QString name = attributes.value("Name").toString();
                    QString sin = attributes.value("SIN").toString();
                    QString min = attributes.value("MIN").toString();

                    cnt = 0;
                    cnt_tu = 1;
                }
                if (xml.name() == "Field"){
                    QXmlStreamAttributes attributes = xml.attributes();

                    QString name = attributes.value("Name").toString();
                    int value = attributes.value("Value").toString().toInt();

                    float data_f = *(float *) &value;

                    if (cnt == 0){
                        epochtime = (int) data_f;

                        const QDateTime time = QDateTime::fromTime_t((int)data_f);
                        dat_time = time.toString("yyyy-MM-dd hh:mm:ss").toLocal8Bit().data();

                        cnt++;
                    }
                    else{
                        q->clear();
                        int id_tu = get.id_tu_ship(q, marine->kapal[n].id_ship, cnt_tu);
                        if (id_tu != 0){
                            q->clear();
                            printf("%d --> %.2f\n", id_tu, data_f);
                            save.data(q, data_f, id_tu, 0, epochtime, dat_time);
                        }
                        else{
                            printf("\nbelum di set parsing refnya");
                        }
                        cnt_tu++;
                    }
                }
            }
        }
    }
#endif
}

void util_skyw::parse_imaniPrima(QString skyw, QSqlDatabase db, utama *marine, account *acc, int id_gateway, int index_gway){
#if 1
    int cnt = 0;
    int cnt_tu = 1;
    int n;

    int SIN;

    int epochtime;
    QString dat_time;

    QString MobileID;
    QString MessageUTC;
    QString UTC_Roll5_Day;
    QString RawPayload;

    QString decode;
    QString bin;
    QString f_5c32g;

    QXmlStreamReader xml;

    QString attributes_name;
    int attributes_SIN;
    int attributes_MIN;
    int tracking_data = 0;

    int f_mUTC;
    int f_5bmUTC;

    xml.clear();
    xml.addData(skyw);

    db.open();
    QSqlQuery q(db);

    while(!xml.atEnd() &&  !xml.hasError()){
        QXmlStreamReader::TokenType token = xml.readNext();
        if(token == QXmlStreamReader::StartElement){
            bool id_match = false;

            if (xml.name() == "MessageUTC"){
                MessageUTC.sprintf("%s", xml.readElementText().toUtf8().data());
                f_mUTC = parse.get_date(MessageUTC);

                strcpy(acc->gway[index_gway].nextutc, MessageUTC.toLatin1());
                tracking_data = 0;
                cnt_df = 0;
#if 0
                uint x = QDateTime::fromString(QString(MessageUTC), "yyyy-MM-dd hh:mm:ss").toTime_t();
                uint y = x - 432000; /* timeStamp 5 day = 432000 -> 3600 * 24 * 5 */

                /* Convert TimeStamp to DateTime */
                QDateTime fiveback =  QDateTime::fromTime_t(y);
                UTC_Roll5_Day = fiveback.toString("yyyy-MM-dd hh:mm:ss");
                f_5bmUTC = parse.get_date(UTC_Roll5_Day);

                bool table_found = get.check_table_is_available(&q, f_5bmUTC);
                qDebug() << "table_found :" << table_found;
                if(table_found){
                    qDebug("Drop tabel data_%d", f_5bmUTC);
                }
#endif
            }
#if 1
            if (xml.name() == "SIN"){
                SIN = xml.readElementText().toInt();
            }


            if(SIN == 128 || SIN == 19){
                bool table_found = get.check_table_is_available(&q, f_mUTC);
                if(!table_found){
                    save.create_tabel_data_harian(&q, f_mUTC);
                }
#if 1
                if (xml.name() == "MobileID"){
                    MobileID.sprintf("%s", xml.readElementText().toUtf8().data());
                }

                /* Filtering Mobile ID */
                for(int i = 0; i < marine->sum_ship; i++){
                    if(marine->kapal[i].modem_id == MobileID){
                        n = i;
                        id_match = true;
                    }
                }

                if(id_match){
                    if(xml.name() == "RawPayload"){
                        printf("\nGet data kapal : %s , mobilde id : %s , Time (UTC) : %s\n",
                               marine->kapal[n].name, marine->kapal[n].modem_id, MessageUTC.toUtf8().data());

                        RawPayload.clear();
                        RawPayload.sprintf("%s", xml.readElementText().toUtf8().data());

                        decode.clear();
                        decode = parse.decode_base64(RawPayload);

                        bin.clear();
                        bin = parse.hex_to_bin_conversion(decode);

                        f_5c32g.clear();
                        f_5c32g = parse.format_5cut_32get(bin);

                        parse.parse_data(&q, f_5c32g, marine->kapal[n].id_ship, f_mUTC, FLAG_ENGINE_DATA);
                    }

                    if(xml.name() == "Payload"){
                        printf("\nGet data tracking : %s , mobilde id : %s , Time (UTC) : %s\n",
                               marine->kapal[n].name, marine->kapal[n].modem_id, MessageUTC.toUtf8().data());
                        QXmlStreamAttributes attributes = xml.attributes();

                        attributes_name = attributes.value("Name").toString();
                        attributes_SIN = attributes.value("SIN").toString().toInt();
                        attributes_MIN = attributes.value("MIN").toString().toInt();

                        if(attributes_name == DATA_TRACKING && attributes_SIN == 19 && attributes_MIN == 4){
                            tracking_data = 1;
                        }
                    }

                    if(xml.name() == "Field"){
                        if(tracking_data){
                            int data_ke = 0;
                            int get_data = 0;
                            int data_float = 0;
                            int id_tu = 0;

                            QXmlStreamAttributes field_attributes = xml.attributes();

                            QString field_name = field_attributes.value("Name").toString();
                            float field_value;
                            int epochTime;

                            if(field_name == "latitude" || field_name == "longitude"){
                                field_value = (float) field_attributes.value("Value").toString().toFloat() / (float) 60000.0;
                                name_df[cnt_df] = field_name;
                                dat_f[cnt_df] = (float) field_value;

                                if(field_name == "latitude") data_ke = 1;
                                else data_ke = 2;

                                get_data = 1;
                                data_float = 1;
                            }
                            else if(field_name == "speed" || field_name == "heading"){
                                field_value = (float) field_attributes.value("Value").toString().toFloat() / (float) 10.0;
                                name_df[cnt_df] = field_name;
                                dat_f[cnt_df] = (float) field_value;

                                if(field_name == "speed") data_ke = 3;
                                else data_ke = 4;

                                get_data = 1;
                                data_float = 1;
                            }
                            else if(field_name == "fixTime"){
                                epochTime = (int) field_attributes.value("Value").toString().toInt();

                                data_ke = 0;
                                get_data = 1;
                                data_float = 0;
                            }

                            if(get_data){
                                if(data_float){
                                   id_tu = get.id_tu_ship(&q, marine->kapal[n].id_ship, data_ke);
                                   tu_df[cnt_df] = id_tu;
                                   cnt_df++;
                                }
                                else{
                                    QString data_raw;

                                    data_raw.clear();

                                    for(int i = 0; i < cnt_df; i++){
                                        const QDateTime time = QDateTime::fromTime_t((((int) epochTime)));

                                        //data_raw.sprintf("%s%d=[%.2f]; ", data_raw.toUtf8().data(), tu_df[i], dat_f[i]);
                                        save.data(&q, dat_f[i], tu_df[i], 0, epochTime, time.toString("yyyy-MM-dd hh:mm:ss").toUtf8().data(), FLAG_TRACKING_DATA);
                                        //save.data_test(&q, dat_f[i], tu_df[i], 0, epochTime, time.toString("yyyy-MM-dd hh:mm:ss").toUtf8().data());
                                        save.data_harian(&q, dat_f[i], tu_df[i], 0, epochTime, time.toString("yyyy-MM-dd hh:mm:ss").toUtf8().data(), f_mUTC, FLAG_TRACKING_DATA);

                                        /* timeStamp 5 day = 432000 -> 3600 * 24 * 5 */
                                        //int epocht_5ago = (int) epochTime - 432000;
                                        //save.delete_data_periodic(&q, epocht_5ago);
                                    }
                                    printf("%s\n", data_raw.toUtf8().data());
                                    cnt_df = 0;
                                }
                            }
                        }
                    }
                    q.clear();
                    save.update_next_utc_gateway(db, MessageUTC, id_gateway);

                    q.clear();
                    save.update_next_utc(db, MessageUTC, marine->kapal[n].id_ship);
                }
#endif
            }
#endif
        }
    }
#endif
}

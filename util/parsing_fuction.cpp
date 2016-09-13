#include "parsing_fuction.h"

parsing_function::parsing_function()
{
}

QString parsing_function::decode_base64(QString data){

    QString text_akhir;
    QByteArray ba;

    ba.append(data);

    text_akhir = QByteArray::fromBase64(ba).toHex();

    return (QString) text_akhir;
}

QString parsing_function::hex_to_bin_conversion(QString hex){
    QString biner = "";

    char dat[hex.size()+1];

    strcpy(dat, hex.toLatin1());

    /* 4 bit pertama di buang, karena hanya sbg penanda
       SIN / MIN dari data SkyWave */

//    for (int i = 4; i < hex.size(); i++){
//        biner.sprintf("%s%s", biner.toLocal8Bit().data(), hex_to_bin(dat[i]).toLocal8Bit().data());
//    }

    for (int i = 0; i < hex.size(); i++){
        biner.sprintf("%s%s", biner.toLocal8Bit().data(), hex_to_bin(dat[i]).toLocal8Bit().data());
    }

    return (QString) biner;
}

QString parsing_function::hex_to_bin(char dat){
    QString biner;

    if (dat == 'F' || dat == 'f') biner = "1111";
    else if (dat == 'E' || dat == 'e') biner = "1110";
    else if (dat == 'D' || dat == 'd') biner = "1101";
    else if (dat == 'C' || dat == 'c') biner = "1100";
    else if (dat == 'B' || dat == 'b') biner = "1011";
    else if (dat == 'A' || dat == 'a') biner = "1010";
    else if (dat == '9') biner = "1001";
    else if (dat == '8') biner = "1000";
    else if (dat == '7') biner = "0111";
    else if (dat == '6') biner = "0110";
    else if (dat == '5') biner = "0101";
    else if (dat == '4') biner = "0100";
    else if (dat == '3') biner = "0011";
    else if (dat == '2') biner = "0010";
    else if (dat == '1') biner = "0001";
    else if (dat == '0') biner = "0000";

    return (QString) biner;
}

QString parsing_function::bin_to_hex_conversion(QString bin){
    QString hexa = "";
    QString temp;

    char dat[bin.size()+1];

    strcpy(dat, bin.toLatin1());

    for (int i = 0; i < bin.size(); i+=4){
        temp.sprintf("%c%c%c%c", bin.at(i), bin.at(i+1), bin.at(i+2), bin.at(i+3));
        hexa.sprintf("%s%s", hexa.toLocal8Bit().data(), bin_to_hex(temp).toLocal8Bit().data());
    }

    return (QString) hexa;
}

QString parsing_function::bin_to_hex(QString dat){
    QString hexa;

    if (dat == "1111") hexa = 'F';
    if (dat == "1110") hexa = 'E';
    if (dat == "1101") hexa = 'D';
    if (dat == "1100") hexa = 'C';
    if (dat == "1011") hexa = 'B';
    if (dat == "1010") hexa = 'A';
    if (dat == "1001") hexa = '9';
    if (dat == "1000") hexa = '8';
    if (dat == "0111") hexa = '7';
    if (dat == "0110") hexa = '6';
    if (dat == "0101") hexa = '5';
    if (dat == "0100") hexa = '4';
    if (dat == "0011") hexa = '3';
    if (dat == "0010") hexa = '2';
    if (dat == "0001") hexa = '1';
    if (dat == "0000") hexa = '0';

    return (QString) hexa;
}

QString parsing_function::format_5cut_32get(QString biner){

    char dat[(biner.size()+1)];

    int c_5bit  = 0;
    int c_32bit = 0;

    bool buang_5bit  = false;
    bool ambil_32bit = false;

    QString dat_5bit  = ""; /* 5 bit yang di buang */
    QString dat_32bit = ""; /* 32 bit yang di ambil */
    QString all_32bit = ""; /* gabung semua 32 bit */

    strcpy(dat, biner.toLatin1());

    for (int i = 0; i < biner.size(); i++){
        if (buang_5bit == false){
            dat_5bit.sprintf("%s%c", dat_5bit.toLocal8Bit().data(), dat[i]);

            if (c_5bit == 4){
                buang_5bit = true;
                c_32bit = 0;
                dat_5bit = "";
            }
            c_5bit++;
        }
        else if(buang_5bit == true){
            dat_32bit.sprintf("%s%c", dat_32bit.toLocal8Bit().data(), dat[i]);

            if (c_32bit == 31){
                all_32bit.sprintf("%s%s", all_32bit.toLocal8Bit().data(), dat_32bit.toLocal8Bit().data());

                buang_5bit = false;
                c_5bit = 0;
                dat_32bit = "";
            }
            c_32bit++;
        }
    }
    return (QString) all_32bit;
}

void parsing_function::parse_data(QSqlQuery *q, QString dat, int id_ship, int f_mUTC, int flag){
    QString data_raw;
    int epochtime;
    QString dat_time;

    float data_f;
    int cnt_p = 0;
    int cnt_d = 0;
    int decimal;
    QString data = "";

    bool ambil_data = false;

    char dats[dat.size()+1];
    strcpy(dats, dat.toLatin1());

    data_raw.clear();

    index = 0;

    for (int i = 0; i < dat.size(); i++){
        cnt_p++;
        data.sprintf("%s%c", data.toLocal8Bit().data(), dats[i]);

        if (cnt_p == 32){
            cnt_d++;

            decimal = bin_to_decimal(data);
            data_f = *(float *) &decimal;

            /* hasil parsing n data float */
            if (cnt_d == 1){
                epochtime = (int) data_f;
                const QDateTime time = QDateTime::fromTime_t((((int)data_f)));
                dat_time = time.toString("yyyy-MM-dd hh:mm:ss").toLocal8Bit().data();
            }
            else{
                q->clear();
                int id_tu = get.id_tu_ship(q, id_ship, cnt_d-1);
                if (id_tu != 0){

                    data_raw.sprintf("%s%d=[%.2f]; ", data_raw.toUtf8().data(), id_tu, data_f);
#if 0
                    q->clear();
                    save.data(q, data_f, id_tu, 0, epochtime, dat_time);
#else
                    measurement_point[index] = id_tu;
                    data_vtes[index] = (float) data_f;

                    if(index > 3 && ambil_data == false){
                        if(data_f != 0.00){
                            ambil_data = true;
                        }
                    }

                    index++;
#endif
                }
                else{
                    printf("\nbelum di set parsing_ref nya");
                }
            }
            data = "";
            cnt_p = 0;
        }
    }

    if(ambil_data){
        for (int i = 0; i < index; i++){
            save.data_skywave(q, data_vtes[i], measurement_point[i], 0, epochtime, dat_time, flag);
            //save.data_test(q, data_vtes[i], measurement_point[i], 0, epochtime, dat_time);
            save.data_harian_skywave(q, data_vtes[i], measurement_point[i], 0, epochtime, dat_time, f_mUTC, flag);

            //int epoch_5ago = (int) epochtime - 432000; /* timeStamp 5 day = 432000 -> 3600 * 24 * 5 */
            //save.delete_data_periodic(q, epoch_5ago);
        }
    }

    printf("%s\n", data_raw.toUtf8().data());
}

int parsing_function::bin_to_decimal(QString dat32){
    int length = dat32.size();
    int pangkat = length-1;
    int decimal = 0;

    char x[length+1];
    strcpy(x, dat32.toLatin1());

    for (int i = 0; i < length; i++){
        if (x[i] == '1') {decimal += operasi_pangkat(pangkat);}
        pangkat--;
    }

    return (int) decimal;
}

int parsing_function::operasi_pangkat(int n){
    int hasil = 1;

    for(int i = 0; i < n; i++){
        hasil *= 2;
    }

    return (int) hasil;
}


int parsing_function::get_date(QString mUTC){
    QString f_mUTC;
    bool utc_state = true;
    char dtimes[mUTC.length()+1];

    f_mUTC.clear();

    strcpy(dtimes, mUTC.toLatin1());
    for(int i = 0; i < mUTC.length(); i++){
        if(dtimes[i] == ' '){
            utc_state = false;
        }

        if(utc_state){
            if(dtimes[i] != '-'){
                f_mUTC.sprintf("%s%c", f_mUTC.toUtf8().data(), dtimes[i]);
            }
        }
    }
    return (int) f_mUTC.toInt();
}

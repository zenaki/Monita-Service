#ifndef PARSING_FUCTION_H
#define PARSING_FUCTION_H

#include <QString>
#include <QDebug>
#include <QDateTime>

#include <util/utama.h>

#include <model/get_db.h>
#include <model/save_db.h>

#define  GMT    25200

class parsing_function
{
public:
    parsing_function();

    QString decode_base64(QString data);
    QString hex_to_bin_conversion(QString hex);
    QString hex_to_bin(char dat);
    QString format_5cut_32get(QString biner);

    void parse_data(QSqlQuery *q, QString dat, int id_ship,int f_mUTC, int flag);

    int bin_to_decimal(QString dat32);
    int operasi_pangkat(int n);

    int get_date(QString mUTC);

private:
    get_db get;
    save_db save;

    float data_vtes[MAX_MEASUREMENT_POINT];
    int measurement_point[MAX_MEASUREMENT_POINT];
    int index;

};

#endif // PARSING_FUCTION_H

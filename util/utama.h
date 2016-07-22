#ifndef UTAMA_H
#define UTAMA_H

#include <QString>

#include "QFile"
#include "QTextStream"
#include "QTime"
#include "QDateTime"

#include <QIODevice>

#include <model/init_mysql.h>
#include <model/monita_log.h>

#define  MODEM_KURAYGEO   1
#define  MODEM_IMANIPRIMA 2


#define  DATA_TRACKING    "noEIO"

#define  FLAG_ENGINE_DATA      0
#define  FLAG_TRACKING_DATA    1

#define  MAX_MEASUREMENT_POINT 999

#define REDIS_ADDRESS "127.0.0.1"
#define REDIS_PORT    6379

#define  PERIODE 1000
#define  MODBUS_PERIOD 60

#define DELAY_DB_CONNECT 5000

#define MAX_SOURCE 50

struct tcp_modbus_config {
    char Ip_TcpModbus[16];
    int Port_TcpModbus;
    int slave_id;
    int function_code;
    int start_address;
    int num_of_coils;
};

#endif // UTAMA_H

#ifndef REDIS_H
#define REDIS_H

//#include <model/monita_log.h>

#include <QString>
#include <QStringList>
#include <QFile>
#include <QDir>

extern "C"
{
//    #include "hiredis/hiredis.h"
    #include "3rdparty/hiredis-64bit/hiredis.h"
}
//
class redis
{
public:
    redis();

    QStringList reqRedis(QString command, QString address, int port, int len = 0);
    QStringList eval(QByteArray script, QString address, int port);
    redisReply *r_reply;
    redisContext *r_context;
//    QStringList result;
//    monita_log log;

    QByteArray readLua(QString pth);
};

#endif // REDIS_H

#ifndef REDIS_H
#define REDIS_H

#include <QString>
#include <QStringList>

extern "C"
{
    #include "hiredis/hiredis.h"
}

class redis
{
public:
    redis();

    QStringList reqRedis(QString command, QString address, int port, int len = 0);
    redisReply *r_reply;
    redisContext *r_context;
};

#endif // REDIS_H

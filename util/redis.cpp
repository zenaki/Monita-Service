#include "redis.h"
#include <QDebug>

redis::redis()
{

}

QStringList redis::reqRedis(QString command, QString address, int port, int len)
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
    if ( r_reply->type == REDIS_REPLY_ERROR ) {
        printf( "Error: %s\n", r_reply->str );
//        result.insert(result.length(), r_reply->str);
    } else if ( r_reply->type != REDIS_REPLY_ARRAY ) {
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
//            QThread::msleep(10);
//            printf( "Monita::Redis::Result:%i: %s\n", i,
//                r_reply->element[i]->str );
        }
    }
    freeReplyObject(r_reply);
    redisFree(r_context);
    return result;
}

QStringList redis::eval(QByteArray script, QString keys, QString argv, QString address, int port)
{
    QStringList result;
    r_context = redisConnect(address.toStdString().c_str(), port);
//    qDebug() << script;
//    QString command =  "eval " +
//            QString(script)  + " " +
//            QString::number(len_key) + " " +
//            keys_argv;
//    QByteArray bytes = command.toLocal8Bit();
//    r_reply = (redisReply *) redisCommand( r_context, "EVAL %s %d %s %s %s %s %s ",
//                                           QString(script).toLatin1().constData(),
//                                           len_key,
//                                           keys.toLatin1().constData(),
//                                           argv1.toLatin1().constData(),
//                                           argv2.toLatin1().constData(),
//                                           argv3.toLatin1().constData(),
//                                           argv4.toLatin1().constData());
    QStringList list_argv = argv.split(" ");
    r_reply = (redisReply *) redisCommand( r_context, "EVAL %s %d %s %s %s %s %s ",
                                           QString(script).toLatin1().constData(),
                                           1,
                                           keys.toLatin1().constData(),
                                           list_argv.at(0).toLatin1().constData(),
                                           list_argv.at(1).toLatin1().constData(),
                                           list_argv.at(2).toLatin1().constData(),
                                           list_argv.at(3).toLatin1().constData());

    if ( r_reply->type == REDIS_REPLY_ERROR ) {
        printf( "Error: %s\n", r_reply->str );
        result.insert(result.length(), r_reply->str);
    } else if (r_reply->type == REDIS_REPLY_ARRAY) {
        for (int i = 0; i < r_reply->elements; i++) {
            result.insert(result.length(), r_reply->element[i]->str);
        }
    } else {
        result.insert(result.length(), r_reply->str);
    }
//    cout<<"EVAL: "<< r_reply->str<<endl;

    freeReplyObject(r_reply);
    redisFree(r_context);
    return result;
}

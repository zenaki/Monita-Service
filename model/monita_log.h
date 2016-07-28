#ifndef MONITA_LOG_H
#define MONITA_LOG_H

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDateTime>

class monita_log
{
public:
    monita_log();

//    char *buf_lay;
//    char *buf_dateTime;

    void write(QString path, QString type, QString message);
};

#endif // MONITA_LOG_H

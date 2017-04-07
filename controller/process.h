#ifndef PROCESS_H
#define PROCESS_H

#include <QObject>

#include "util/utama.h"
#include "util/config.h"

class process : public QObject
{
    Q_OBJECT
public:
    explicit process(QObject *parent = 0);

    void doSetup(QThread &cThread, QString plugins_path, QStringList arg, QStringList sn, int time_periode);

signals:

public slots:
    void doWork();

private:
    config cfg;
    redis rds;
    monita_log log;
    struct monita_config monita_cfg;

    QString Plugin;
    QStringList Argv;
    QStringList SN;

    QJsonObject ObjectFromString(QString in);

    bool logsheet;
    void monita_parse(QJsonObject obj, int index);
    void skywave_parse(QJsonObject obj);
};

#endif // PROCESS_H

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

    void doSetup(QThread &cThread, QStringList plugins_id, QString plugins_path, QStringList arg, QStringList sn, int time_periode);

signals:

public slots:
    void doWork();

private:
    config cfg;
    redis rds;
    monita_log log;
    struct monita_config monita_cfg;

    QStringList id;
    QString Plugin;
    QStringList Argv;
    QStringList SN;
    QStringList logsheet;

    QJsonObject ObjectFromString(QString in);

//    bool logsheet;
    void monita_parse(QJsonObject obj, int index);
    void skywave_parse(QJsonObject obj, int index);
};

#endif // PROCESS_H

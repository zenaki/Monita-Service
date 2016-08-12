#ifndef DATA_VISUAL_H
#define DATA_VISUAL_H

#include <QObject>

#include "util/utama.h"
#include "util/config.h"

class data_visual : public QObject
{
    Q_OBJECT
public:
    explicit data_visual(QObject *parent = 0);

    void doSetup(QThread &cThread);

    config cfg;
    redis rds;
    monita_log log;
    struct monita_config monita_cfg;

private:
    void RedisToJson(QStringList data, QDateTime dt);
    void WriteToJson(QJsonObject json, QDateTime dt);

public slots:
    void doWork();
};

#endif // DATA_VISUAL_H

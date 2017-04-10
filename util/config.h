#ifndef CONFIG_H
#define CONFIG_H

#include "util/utama.h"

class config
{
public:
    config();

    enum SaveFormat {
        Json, Binary
    };

    struct plugins plg;

    QStringList read(QString obj);
    void write(QJsonObject &json) const;    //Default
    bool loadConfig(config::SaveFormat saveFormat);
    bool saveConfig(config::SaveFormat saveFormat) const;

//    QStringList get_skywave_parameter();
    plugins get_plugins();
    void set_nextutc_skywave(QString id, QString nextUTC);
private:
    QJsonDocument JsonDoc;
    QByteArray data_json;

    init_mysql mysql;
    plugins get_plugins_from_database(struct plugins plg, int index);
};

#endif // CONFIG_H

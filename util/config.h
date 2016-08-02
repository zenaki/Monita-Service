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

    QStringList read(QString obj);
    void write(QJsonObject &json) const;    //Default
    bool loadConfig(config::SaveFormat saveFormat);
    bool saveConfig(config::SaveFormat saveFormat) const;
private:
    QJsonDocument JsonDoc;
    QByteArray data_json;
};

#endif // CONFIG_H

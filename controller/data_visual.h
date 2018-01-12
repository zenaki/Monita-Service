#ifndef DATA_VISUAL_H
#define DATA_VISUAL_H

#include <QObject>

#include <QWebSocketServer>
#include <QWebSocket>

#include "util/utama.h"
#include "util/config.h"

class data_visual : public QObject
{
    Q_OBJECT
public:
    explicit data_visual(QObject *parent = 0);
    ~data_visual();

    void doSetup(QThread &cThread);

    config cfg;
    redis rds;
    monita_log log;
    struct monita_config monita_cfg;

private:
    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;
    QStringList m_type;
    QStringList m_id;
    QStringList m_nama_titik_ukur;
    QStringList m_titik_ukur;
    QStringList m_type_titik_ukur;
    QStringList m_satuan;
    QStringList m_arguments;

    void RedisToJson(QStringList data, QDateTime dt, int index);
    void WriteToJson(QJsonObject json, QString type, QString id, QDateTime dt, int index);

    init_mysql mysql;
    QSqlDatabase db;

    void get_titik_ukur(QString type, QString id, int index);
    void get_arguments(QString id);
    void exec_arguments();
    void set_customInput(QStringList data);
    QJsonObject ObjectFromString(QString in);
    void generate_report(int index, QString temp, QString conf, QString name, QString parameter);

public slots:
    void doWork();

private Q_SLOTS:
    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();

Q_SIGNALS:
    void closed();
};

#endif // DATA_VISUAL_H

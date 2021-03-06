#include "data_visual.h"

data_visual::data_visual(QObject *parent) : QObject(parent)
{
//    QTimer *t = new QTimer(this);
    //    connect(t, SIGNAL(timeout()), this, SLOT(doWork());
}

data_visual::~data_visual()
{
    if (m_pWebSocketServer->isListening()) {
        m_pWebSocketServer->close();
        qDeleteAll(m_clients.begin(), m_clients.end());
    }
}

void data_visual::doSetup(QThread &cThread)
{
    connect(&cThread,SIGNAL(started()),this,SLOT(doWork()));

    monita_cfg.config = cfg.read("CONFIG");
    int port = monita_cfg.config.at(5).toInt();
    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("WebSocket Server"), QWebSocketServer::NonSecureMode, this);
    if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
        log.write("WebSocket","Server listening on port : " + QString::number(port));
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection,this, &data_visual::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &data_visual::closed);
    }

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(doWork()));
    t->start(500);
}

void data_visual::RedisToJson(QStringList data, QDateTime dt)
{
    monita_cfg.source_config = cfg.read("SOURCE");
    monita_cfg.calc_config = cfg.read("CALC");
    monita_cfg.funct_config = cfg.read("FUNCT");
    if (monita_cfg.source_config.length() > 9) {monita_cfg.jml_sumber = monita_cfg.source_config.length()/9;
    } else {monita_cfg.jml_sumber++;}
    if (!monita_cfg.calc_config.isEmpty()) {monita_cfg.jml_sumber = monita_cfg.jml_sumber + (monita_cfg.calc_config.length()/4);}
    if (!monita_cfg.funct_config.isEmpty()) {monita_cfg.jml_sumber = monita_cfg.jml_sumber + (monita_cfg.funct_config.length()/3);}

    QJsonObject json;
    QJsonArray slaveArray[monita_cfg.jml_sumber];
    QJsonObject idTitikUkurObject[monita_cfg.jml_sumber];

    QStringList list_temp; QString temp; int index = 0;
    for (int i = 0; i < data.length(); i+=2) {
        list_temp = data.at(i).split(";");
        if ((i > 0 && temp != list_temp.at(0))) {
            slaveArray[index].append(idTitikUkurObject[index]);
            json["SLAVE_ID:"+temp] = slaveArray[index];
            index++;
        }
        idTitikUkurObject[index]["ID_TITIK_UKUR:"+list_temp.at(1)] = data.at(i+1);
        temp = list_temp.at(0);
        if (i >= data.length()-2) {
            slaveArray[index].append(idTitikUkurObject[index]);
            json["SLAVE_ID:"+temp] = slaveArray[index];
        }
    }

    this->WriteToJson(json, dt);
}

void data_visual::WriteToJson(QJsonObject json, QDateTime dt)
{
    QString path = ".MonSerConfig/VisMon_" + dt.date().toString("dd_MM_yyyy") + ".json";
    QFile visual_json_file(path);
    if (!visual_json_file.exists()) {
        QDir dir;
        dir.mkpath(".MonSerConfig");
    }/* else {
        visual_json_file.remove();
        QDir dir;
        dir.mkpath(".MonSerConfig");
    }*/
    if (visual_json_file.open(QIODevice::ReadWrite|QIODevice::Truncate)) {
        QJsonDocument saveDoc(json);
        visual_json_file.write(saveDoc.toJson());
    }
    visual_json_file.close();
    if (visual_json_file.open(QIODevice::ReadWrite)) {
        QByteArray readFile = visual_json_file.readAll();
        for (int i = 0; i < m_clients.length(); i++) {
            m_clients.at(i)->sendTextMessage(readFile);
        }
    }
}

void data_visual::doWork()
{
    monita_cfg.config = cfg.read("CONFIG");
    QStringList redis_config = cfg.read("REDIS");
    QString address = redis_config.at(0);
    int port = redis_config.at(1).toInt();
    QDateTime dt = QDateTime::currentDateTime();
    QStringList request = rds.reqRedis("hlen monita_service:vismon", address, port);
    log.write("Redis",request.at(0) + " Data ..");
    int redis_len = request.at(0).toInt();
    request = rds.reqRedis("hgetall monita_service:vismon", address, port, redis_len*2);
//    if (request.length() > 0) {
//        this->RedisToJson(request, dt);
//        request = rds.reqRedis("del monita_service:vismon", address, port, redis_len*2);
//    }
    this->RedisToJson(request, dt);
//    request = rds.reqRedis("del monita_service:vismon", address, port, redis_len*2);
}

void data_visual::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &data_visual::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &data_visual::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &data_visual::socketDisconnected);

    pSocket->ignoreSslErrors();
    log.write("WebSocket","Socket Connect : " + pSocket->localAddress().toString() + ":" + pSocket->localPort());
    pSocket->sendTextMessage("Berhasil Connect cuy ..");

    m_clients << pSocket;
}

void data_visual::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    log.write("WebSocket","Message received : " + message);
    if (pClient) {pClient->sendTextMessage(message);}
}

void data_visual::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    log.write("WebSocket","Binary Message received : " + message);
    if (pClient) {pClient->sendBinaryMessage(message);}
}

void data_visual::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    log.write("WebSocket","Socket Disconnect : " + pClient->localAddress().toString() + ":" + pClient->localPort());
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}


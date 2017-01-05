#include "worker.h"

//#include "modbus-tcp.h"

Worker::Worker(QObject *parent) : QObject(parent)
{   
//    manager = new QNetworkAccessManager();
//    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply *)));

//    monita_cfg.config = cfg.read("CONFIG");
//    port = monita_cfg.config.at(5).toInt();
//    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("WebSocket Server"), QWebSocketServer::NonSecureMode, this);
//    if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
//        log.write("WebSocket","Server listening on port : " + QString::number(port));
//        connect(m_pWebSocketServer, &QWebSocketServer::newConnection,this, &Worker::onNewConnection);
//        connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &Worker::closed);
//    }

//    connect(&timer, SIGNAL(timeout()), this, SLOT(doWork()));
//    timer.start(monita_cfg.config.at(1).toInt());

//    monita_cfg.modbus_period = 0;

//    marine = (struct sky_wave_ship *) malloc( sizeof (struct sky_wave_ship));
//    memset((char *) marine, 0, sizeof(struct sky_wave_ship));

//    acc = (struct sky_wave_account *) malloc ( sizeof (struct sky_wave_account));
//    memset((char *) acc, 0, sizeof(struct sky_wave_account));

//    get.modem_info(db, marine);
//    get.modem_getway(db, acc);

//    ship_count = 0;
//    gateway_count = 0;
//    cnt_panggil = 0;

    this->doWork();
}

Worker::~Worker()
{
    if (ThreadTcpModbus.isRunning()) ThreadTcpModbus.terminate();
    if (ThreadDataMysql.isRunning()) ThreadDataMysql.terminate();
    if (ThreadDataVisual.isRunning()) ThreadDataVisual.terminate();
    if (ThreadSkyWave.isRunning()) ThreadSkyWave.terminate();
//    if (m_pWebSocketServer->isListening()) {
//        m_pWebSocketServer->close();
//        qDeleteAll(m_clients.begin(), m_clients.end());
//    }
}

void Worker::doWork()
{
    obj_tcp_modbus.doSetup(ThreadTcpModbus);
    obj_tcp_modbus.moveToThread(&ThreadTcpModbus);
    ThreadTcpModbus.start();

    obj_data_mysql.doSetup(ThreadDataMysql);
    obj_data_mysql.moveToThread(&ThreadDataMysql);
    ThreadDataMysql.start();

    obj_data_visual.doSetup(ThreadDataVisual);
    obj_data_visual.moveToThread(&ThreadDataVisual);
    ThreadDataVisual.start();

    obj_sky_wave.doSetup(ThreadSkyWave);
    obj_sky_wave.moveToThread(&ThreadSkyWave);
    ThreadSkyWave.start();

//    this->request_sky_wave();
}

//void Worker::request_sky_wave()
//{
//    QNetworkRequest request;

//    monita_cfg.urls.sprintf("%s%s", acc->gway[monita_cfg.gateway_count].link, acc->gway[monita_cfg.gateway_count].nextutc);
//    QUrl url =  QUrl::fromEncoded(monita_cfg.urls.toLocal8Bit().data());

//    request.setUrl(url);
//    manager->get(request);
//}

//void Worker::replyFinished(QNetworkReply* reply){
//    QString xmlStr;
//    xmlStr.clear();

//    xmlStr=reply->readAll();
//    read.parse_xml_account_methode(xmlStr, db, marine, acc, acc->gway[monita_cfg.gateway_count].id, monita_cfg.gateway_count);

//    monita_cfg.gateway_count++;

//    if (monita_cfg.gateway_count < acc->sum_getway){
//        this->request_sky_wave();
//    }
//    else{
//        monita_cfg.gateway_count = 0;
////        timer.start((1000 * 60 * 10) / 2);
//    }
//}

//void Worker::onNewConnection()
//{
//    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

//    connect(pSocket, &QWebSocket::textMessageReceived, this, &Worker::processTextMessage);
//    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &Worker::processBinaryMessage);
//    connect(pSocket, &QWebSocket::disconnected, this, &Worker::socketDisconnected);

//    pSocket->ignoreSslErrors();
//    log.write("WebSocket","Socket Connect : " + pSocket->localAddress().toString() + ":" + pSocket->localPort());
//    pSocket->sendTextMessage("Berhasil Connect cuy ..");

//    m_clients << pSocket;
//}

//void Worker::processTextMessage(QString message)
//{
//    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
//    log.write("WebSocket","Message received : " + message);
//    if (pClient) {pClient->sendTextMessage(message);}
//}

//void Worker::processBinaryMessage(QByteArray message)
//{
//    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
//    log.write("WebSocket","Binary Message received : " + message);
//    if (pClient) {pClient->sendBinaryMessage(message);}
//}

//void Worker::socketDisconnected()
//{
//    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
//    log.write("WebSocket","Socket Disconnect : " + pClient->localAddress().toString() + ":" + pClient->localPort());
//    if (pClient) {
//        m_clients.removeAll(pClient);
//        pClient->deleteLater();
//    }
//}

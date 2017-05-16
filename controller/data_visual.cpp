#include "data_visual.h"

data_visual::data_visual(QObject *parent) : QObject(parent)
{
//    QTimer *t = new QTimer(this);
    //    connect(t, SIGNAL(timeout()), this, SLOT(doWork());

    db = mysql.connect_db("VisMon");
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

    bool webSocketServer = false;
    while (!webSocketServer) {
        if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
            log.write("WebSocket","Server listening on port : " + QString::number(port),
                      monita_cfg.config.at(7).toInt());
            connect(m_pWebSocketServer, &QWebSocketServer::newConnection,this, &data_visual::onNewConnection);
            connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &data_visual::closed);
            webSocketServer = true;
        } else {
            m_pWebSocketServer->close();
        }
    }

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(doWork()));
    t->start(500);
}

void data_visual::RedisToJson(QStringList data, QDateTime dt, int index)
{
    QJsonObject json;
//    QJsonArray slaveArray[JUMLAH_MAX_TITIK_UKUR];            // Jumlah Maksimal titik Ukut
//    QJsonObject idTitikUkurObject[JUMLAH_MAX_TITIK_UKUR];    // Jumlah Maksimal titik Ukut
//    QJsonArray slaveArray;
//    QJsonObject idTitikUkurObject;
//    QJsonObject errorObject;
//    QJsonObject dataObject;
//    QJsonArray dataArray;
    QJsonObject VisMonObject;
    QJsonArray VisMonArray;

    QStringList list_temp;
    QStringList list_temp2;
//    QString temp; int index = 0; int indexObject = 0;
//    for (int i = 0; i < data.length(); i+=2) {
//        if (data.at(i).indexOf("UNKNOWN") > 0) {
//            errorObject[data.at(i)] = data.at(i+1);
//            json[data.at(i)] = errorObject;
//        } else {
//            list_temp = data.at(i).split(";");
//            if ((i > 0 && temp != list_temp.at(0))) {
//                slaveArray[index].append(idTitikUkurObject[index]);
//                json["SLAVE_ID:"+temp] = slaveArray[index];
//                index++;
//            }
//            idTitikUkurObject[index]["ID_TITIK_UKUR:"+list_temp.at(1)] = data.at(i+1);
//            temp = list_temp.at(0);
//            if (i >= data.length()-2) {
//                slaveArray[index].append(idTitikUkurObject[index]);
//                json["SLAVE_ID:"+temp] = slaveArray[index];
//            }
//        }
//    }

//    for (int i = 0; i < data.length(); i+=2) {
//        if (data.at(i).indexOf("UNKNOWN") > 0) {
//            errorObject[data.at(i)] = data.at(i+1);
//            json[data.at(i)] = errorObject;
//        } else {
//            list_temp = data.at(i).split(";");
//            if ((i > 0 && temp != list_temp.at(0))) {
////                slaveArray[index].append(idTitikUkurObject[index]);
////                slaveArray[index].append(dataArray);
//                json["slave_id"] = temp.toInt();
//                json["data"] = dataArray;
//                index++;
//                for (int j = 0; j < indexObject; j++) {
//                    dataArray.removeAt(0);
//                }
//                indexObject = 0;
//                VisMonArray.append(json);
//            }
//            if (data.at(i+1) != "N/A") {
//                dataObject["titik_ukur"] = list_temp.at(1);
//                dataObject["value"] = data.at(i+1);
////                idTitikUkurObject[index]["ID_TITIK_UKUR:"+list_temp.at(1)] = data.at(i+1);
////                idTitikUkurObject[indexObject] = dataObject;
//                dataArray.append(dataObject); indexObject++;
//            }
//            temp = list_temp.at(0);
//            if (i >= data.length()-2) {
////                slaveArray[index].append(idTitikUkurObject[index]);
////                slaveArray[index].append(dataArray);
//                json["slave_id"] = temp.toInt();
//                json["data"] = dataArray;
//                VisMonArray.append(json);
//            }
//        }
//    }

    for (int i = 0; i < data.length(); i+=2) {
        QStringList list_titik_ukur = m_titik_ukur.at(index).split(";");
        list_temp = data.at(i).split(";");
        list_temp2 = data.at(i+1).split(";");
        for (int j = 0; j < list_titik_ukur.length(); j++) {
            if (list_temp.at(1) == list_titik_ukur.at(j)) {
                json["serial_number"] = list_temp.at(0);
                json["titik_ukur"] = list_temp.at(1);
//                json["value"] = data.at(i+1);
//                json["epochtime"] = QString::number(QDateTime::currentMSecsSinceEpoch());
                json["epochtime"] = list_temp2.at(0);
                json["value"] = list_temp2.at(1);
                if (m_nama_titik_ukur.at(index).split(";").length() > j) {
                    json["nama_tu"] = m_nama_titik_ukur.at(index).split(";").at(j);
                } else {
                    json["nama_tu"] = "";
                }
//                json["epochtime"] = list_temp.at(2);
                VisMonArray.append(json);
            }
        }
    }
    VisMonObject["monita"] = VisMonArray;
    this->WriteToJson(VisMonObject, m_type.at(index), m_id.at(index), dt, index);
}

void data_visual::WriteToJson(QJsonObject json, QString type, QString id, QDateTime dt, int index)
{
    QString path = ".MonSerConfig/VisMon_" + type + "_" + id + "_" + dt.date().toString("yyyyMMdd") + ".json";
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
        m_clients.at(index)->sendTextMessage(readFile);
//        for (int i = 0; i < m_clients.length(); i++) {
//            if (m_clients.at(i)->isValid()) {
//                m_clients.at(i)->sendTextMessage(readFile);
//            }
//        }
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
//    log.write("Redis",request.at(0) + " Data ..",
//              monita_cfg.config.at(7).toInt());
    int redis_len = request.at(0).toInt();
    if (redis_len > 0) {
        request = rds.reqRedis("hgetall monita_service:vismon", address, port, redis_len*2);

        for (int i = 0; i < m_clients.length(); i++) {
            this->RedisToJson(request, dt, i);
        }
//        request = rds.reqRedis("del monita_service:vismon", address, port, redis_len*2);
    }
}

void data_visual::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &data_visual::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &data_visual::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &data_visual::socketDisconnected);

    pSocket->ignoreSslErrors();
    log.write("WebSocket","New Client : " + pSocket->localAddress().toString() + ":" + pSocket->localPort(),
              monita_cfg.config.at(7).toInt());
//    pSocket->sendTextMessage("Berhasil Connect cuy ..");

    m_clients << pSocket;
    m_type << "";
    m_id << "";
    m_nama_titik_ukur << "";
    m_titik_ukur << "";
//    qDebug() << "debug new webSockcet Server Connection";

    monita_cfg.config = cfg.read("CONFIG");
    QStringList redis_config = cfg.read("REDIS");
    QString address = redis_config.at(0);
    int port = redis_config.at(1).toInt();
    QStringList request = rds.reqRedis("hlen monita_service:vismon", address, port);
    int redis_len = request.at(0).toInt();
    request = rds.reqRedis("del monita_service:vismon", address, port, redis_len*2);
}

void data_visual::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    log.write("WebSocket","Client : " + pClient->localAddress().toString() + " Message received : " + message,
              monita_cfg.config.at(7).toInt());

//    if (pClient) {pClient->sendTextMessage(message);}
    for (int i = 0; i < m_clients.length(); i++) {
        if (m_clients.at(i) == pClient) {
            if (message.split(':').length() == 2) {
                m_type.replace(i, message.split(':').at(0));
                m_id.replace(i, message.split(':').at(1));
                if (m_type.at(i) == "id") {
                    this->get_titik_ukur(m_type.at(i), m_id.at(i), i);
                } else if (m_type.at(i) == "op") {
                    QString resultTitikUkur = m_id.at(i);
                    m_nama_titik_ukur.replace(i, "");
                    m_titik_ukur.replace(i, resultTitikUkur);
                    m_id.replace(i, "");
                }
            }
            break;
        }
    }
}

void data_visual::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    log.write("WebSocket","Client : " + pClient->localAddress().toString() + " Binary Message received : " + message,
              monita_cfg.config.at(7).toInt());
    if (pClient) {pClient->sendBinaryMessage(message);}

}

void data_visual::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    log.write("WebSocket","Socket Disconnect : " + pClient->localAddress().toString() + ":" + pClient->localPort(),
              monita_cfg.config.at(7).toInt());
    if (pClient) {
        for (int i = 0; i < m_clients.length(); i++) {
            if (m_clients.at(i) == pClient) {
                m_nama_titik_ukur.removeAt(i);
                m_titik_ukur.removeAt(i);
                break;
            }
        }
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

void data_visual::get_titik_ukur(QString type, QString id, int index) {
    QString resultNama;
    QString resultTitikUkur;
    db.open();
    QSqlQuery q(QSqlDatabase::database(db.connectionName()));

//    if(!q.exec("call get_titik_ukur('" + type + "', " + id + ");")){
    if(!q.exec("call get_titik_ukur(" + id + ");")){
        return;
    }
    else{
        while(q.next()){
            resultNama = resultNama + q.value(0).toString().toLatin1() + ";";
            resultTitikUkur = resultTitikUkur + q.value(1).toString().toLatin1() + ";";
        }
    }

    m_nama_titik_ukur.replace(index, resultNama);
    m_titik_ukur.replace(index, resultTitikUkur);

    db.close();
}


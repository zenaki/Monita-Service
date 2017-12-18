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
    QJsonObject VisMonObject;
    QJsonArray VisMonArray;

    QStringList list_temp;
    QStringList list_temp2;

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
//                    json["nama_tu"] = "";
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
        visual_json_file.close();
        visual_json_file.remove();
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

    QStringList request = rds.reqRedis("hlen monita_service:realtime", address, port);
//    log.write("Redis",request.at(0) + " Data ..",
//              monita_cfg.config.at(7).toInt());
    if (request.isEmpty()) return;
    int redis_len = request.at(0).toInt();
    if (redis_len > 0) {
        request = rds.reqRedis("hgetall monita_service:realtime", address, port, redis_len*2);
//        for (int i = 0; i < request.length(); i++) {
//            log.write("RealTIme", request.at(i), 0);
//        }
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

//    monita_cfg.config = cfg.read("CONFIG");
//    QStringList redis_config = cfg.read("REDIS");
//    QString address = redis_config.at(0);
//    int port = redis_config.at(1).toInt();
//    QStringList request = rds.reqRedis("hlen monita_service:vismon", address, port);
//    int redis_len = request.at(0).toInt();
//    request = rds.reqRedis("del monita_service:vismon", address, port, redis_len*2);
}

void data_visual::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    log.write("WebSocket","Client : " + pClient->localAddress().toString() + " Message received : " + message,
              monita_cfg.config.at(7).toInt());

//    if (pClient) {pClient->sendTextMessage(message);}
    for (int i = 0; i < m_clients.length(); i++) {
        if (m_clients.at(i) == pClient) {
            if (message.split(':').length() >= 2) {
                if (message.split(':').at(0) == "id") {
                    m_type.replace(i, message.split(':').at(0));
                    m_id.replace(i, message.split(':').at(1));
                    this->get_titik_ukur(m_type.at(i), m_id.at(i), i);
                } else if (message.split(':').at(0) == "op") {
                    m_type.replace(i, message.split(':').at(0));
                    m_id.replace(i, message.split(':').at(1));
                    QString resultTitikUkur = m_id.at(i);
                    m_nama_titik_ukur.replace(i, "");
                    m_titik_ukur.replace(i, resultTitikUkur);
                    m_id.replace(i, "");
                } else if (message.split(':').at(0) == "arg") {
                    get_arguments(message.split(':').at(1));
                    exec_arguments();
                } else if (message.split(':').at(0) == "rpt") {
                    QStringList list_temp = message.split(':');
                    if (list_temp.length() >= 4) {
//                        rpt:template.xml:config.json:path_name:parameter1#parameter2#parameter3#...
//                        rpt:/home/ovm/sample_report.xml:/home/ovm/sample_config.json:/home/ovm/sample.pdf:
//                        rpt:/home/zenaki/Desktop/sample_report.xml:/home/zenaki/Desktop/sample_config.json:/home/zenaki/Desktop/sample.pdf:
                        this->generate_report(i, list_temp.at(1), list_temp.at(2), list_temp.at(3), list_temp.at(4));
                    }
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
//    log.write("Test", "type : " + type + " id : " + id, 0);
    Q_UNUSED(type);
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

void data_visual::get_arguments(QString id) {
    QStringList result;
    db.open();
    QSqlQuery q(QSqlDatabase::database(db.connectionName()));
    if (!q.exec("call get_arguments("+id+")")) {
        return;
    } else {
        while(q.next()){
            result.append(q.value(2).toString().toLatin1());
        }
    }

    m_arguments = result;
}

void data_visual::exec_arguments() {
    QStringList result;
    for (int i = 0; i < m_arguments.length(); i++) {
        if (m_arguments.at(i).indexOf("ARG") > 0) {
            QString temp = m_arguments.at(i).mid(m_arguments.at(i).indexOf("ARG"));
            QString temp2;
            if (temp.indexOf("#") > 0) {
                temp2 = temp.mid(0, temp.indexOf("#"));
            } else {
                temp2 = temp.mid(0, temp.indexOf(" "));
            }
            QStringList tempList =  temp2.split("_");
            if (i > tempList.at(1).toInt()-1) {
//                QString().toUtf8().data();
                temp = m_arguments.at(i);
                temp.replace(temp2, result.at(tempList.at(1).toInt()-1));
                m_arguments.replace(i, temp);
            }
            qDebug() << temp;
        }
        QProcess proc;
        proc.start(m_arguments.at(i));
        proc.waitForFinished();
        QString output(proc.readAllStandardOutput());
        QJsonObject obj = this->ObjectFromString(output);

        if (!obj.value("success").isUndefined()) {
            if (obj.value("success").toString() == "false") {
                if (!obj.value("ERR").isUndefined()) {
                   qDebug() << "----------------------------------------------------------------------------------------------------";
                   qDebug() << "----------------------------------------------------------------------------------------------------";
                   qDebug() << "----------------------------------------------------------------------------------------------------";
                   qDebug() << "EXEC_ARG:" + output;
                   qDebug() << "----------------------------------------------------------------------------------------------------";
                   qDebug() << "----------------------------------------------------------------------------------------------------";
                   qDebug() << "----------------------------------------------------------------------------------------------------";
               }
                break;
            } else {
                if (!obj.value("monita").isUndefined()) {
                    QJsonArray array = obj.value("monita").toArray();
                    foreach (const QJsonValue & v, array) {
                        result.append(v.toObject().value("value").toString());
                    }
                } else {
                    qDebug() << "----------------------------------------------------------------------------------------------------";
                    qDebug() << "----------------------------------------------------------------------------------------------------";
                    qDebug() << "----------------------------------------------------------------------------------------------------";
                    qDebug() << "EXEC_ARG:" + output;
                    qDebug() << "----------------------------------------------------------------------------------------------------";
                    qDebug() << "----------------------------------------------------------------------------------------------------";
                    qDebug() << "----------------------------------------------------------------------------------------------------";
                    break;
                }
            }
        }
    }
}

QJsonObject data_visual::ObjectFromString(QString in)
{
    QJsonObject obj;
    QJsonDocument doc = QJsonDocument::fromJson(in.toUtf8());

    // check validity of the document
    if (!doc.isNull()) {
        if (doc.isObject()) {
            obj = doc.object();
        }
    }

    return obj;
}

void data_visual::generate_report(int index, QString temp, QString conf, QString name, QString parameter)
{
    QSettings db_sett(PATH_DB_CONNECTION, QSettings::IniFormat);
    QString host = db_sett.value("HOST").toString();
    QString db_name = db_sett.value("DATABASE").toString();
    QString user_name = db_sett.value("USERNAME").toString();
    QString password = db_sett.value("PASSWORD").toString();

    QStringList rpt_gen = cfg.read("RPT_GEN");

    log.write("Process", rpt_gen.at(0) +
              " -tmp " + temp +
              " -cnf " + conf +
              " -f " + name +
              " -host " + host +
              " -db " + db_name +
              " -usr " + user_name +
              " -pwd " + password +
              " -par " + parameter, monita_cfg.config.at(6).toInt());
    QProcess proc;
    proc.start("xvfb-run -- /" + rpt_gen.at(0) +
               " -tmp " + temp +
               " -cnf " + conf +
               " -f " + name +
               " -host " + host +
               " -db " + db_name +
               " -usr " + user_name +
               " -pwd " + password +
               " -par " + parameter);
    proc.waitForFinished(); // sets current thread to sleep and waits for pingProcess end
    QString output(proc.readAllStandardOutput());
//    for (int i = 0; i < 1000; i++) {
//        qDebug() << output;
//    }
    output.mid(output.indexOf("{"));
    QJsonObject obj = this->ObjectFromString(output);
    if (obj.value("ERR").isUndefined()) {
        m_clients.at(index)->sendTextMessage("{\"REPORT\": \""+ obj.value("SUCCESS").toString() +"\"}");
    } else {
        m_clients.at(index)->sendTextMessage("{\"REPORT\": \""+ obj.value("ERR").toString() +"\"}");
    }
}

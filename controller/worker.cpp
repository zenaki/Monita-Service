#include "worker.h"

Worker::Worker(QObject *parent) : QObject(parent)
{   
    this->doWork();
}

Worker::~Worker()
{
    for (int i = 0; i < jml_thread; i++) {
        if (ThreadApp[i].isRunning()) {
            ThreadApp[i].terminate();
        }
    }
    
    if (ThreadDataMysql.isRunning()) ThreadDataMysql.terminate();
    if (ThreadDataVisual.isRunning()) ThreadDataVisual.terminate();
//    if (ThreadHttpServer.isRunning()) ThreadHttpServer.terminate();
}

void Worker::doWork()
{
//    QProcess tcp_modbus;
//    tcp_modbus.start("./ModBus -tcp -ip 192.168.3.21 -p 502 -s 1 -f 3 -str 1000 -noc 20 -nob 2 -t FLOAT");
//    tcp_modbus.waitForFinished(); // sets current thread to sleep and waits for pingProcess end
//    QString output(tcp_modbus.readAllStandardOutput());
//    QJsonObject obj = this->ObjectFromString(output);
//    QStringList result = output.split("\n");

    plg = cfg.get_plugins();
    jml_thread = 0;
    for (int i = 0; i < MAX_PLUGINS; i++) {
        if (plg.arg[i].length() > 0) {
            obj_app[jml_thread].doSetup(ThreadApp[jml_thread], plg.id[i], plg.path[i], plg.arg[i], plg.sn[i], plg.time_periode[i]);
            obj_app[jml_thread].moveToThread(&ThreadApp[jml_thread]);
            ThreadApp[jml_thread].start();
            jml_thread++;
        }
    }
    printf("%s::Total Thread = %d\n",
           QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss").toLatin1().data(),
           jml_thread);

//    log.write("Test", "Start Thread Data MySQL", 0);
    obj_data_mysql.doSetup(ThreadDataMysql);
    obj_data_mysql.moveToThread(&ThreadDataMysql);
    ThreadDataMysql.start();

//    log.write("Test", "Start Thread Data Visual", 0);
    obj_data_visual.doSetup(ThreadDataVisual);
    obj_data_visual.moveToThread(&ThreadDataVisual);
    ThreadDataVisual.start();

//    obj_http_server.doSetup(ThreadHttpServer);
//    obj_http_server.moveToThread(&ThreadHttpServer);
//    ThreadHttpServer.start();
}

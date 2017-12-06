#include "http_server.h"

http_server::http_server(QObject *parent) : QObject(parent)
{

}

void http_server::doSetup(QThread &cThread)
{
    connect(&cThread,SIGNAL(started()),this,SLOT(doWork()));
    monita_cfg.config = cfg.read("CONFIG");
}

void http_server::doWork()
{
    qttp::HttpServer* httpSvr = qttp::HttpServer::getInstance();
    httpSvr->initialize();

    auto action = httpSvr->addAction<api>();
    action->registerRoute(qttp::HttpMethod::GET, "kolek");
    action->registerRoute(qttp::HttpMethod::PUT, "kolek");

    qttp::HttpServer::ServerInfo info = httpSvr->getServerInfo();
    info.title = "Monita Service HTTP Server";
    info.description = "HTTP Server for Rinjani API";
    info.companyName = "PT. Daun Biru Engineering Indonesia";
    httpSvr->setServerInfo(info);

    monita_cfg.config = cfg.read("CONFIG");
    httpSvr->startServer("0.0.0.0", monita_cfg.config.at(8).toInt());
}

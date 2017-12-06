#include "api.h"

api::api()
{

}

void api::onGet(qttp::HttpData& data)
{
//    qDebug() << data.getRequest().getQuery().toString();
    QStringList parameter = data.getRequest().getQuery().toString().split("&");
    qttp::HttpResponse& response = data.getResponse();
    QJsonObject& json = response.getJson();
    json["parameter"] = QJsonArray::fromStringList(parameter);
//    getParameter(parameter);
//    qDebug() << "Test";
}

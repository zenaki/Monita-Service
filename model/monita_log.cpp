#include "monita_log.h"

monita_log::monita_log()
{
//    buf_lay = (char *) malloc(1024);
//    buf_dateTime = (char *) malloc(32);
}

void monita_log::write(QString path, QString type, QString message)
{
    QFile outFile(path);
    if (!outFile.exists()) {
        QDir dir;
        dir.mkpath("monita_configuration");
    }
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    message.prepend("::").prepend(type).prepend("::").prepend(QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss"));
//    message.prepend("::").prepend(QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss"));
    qDebug() << message;
    ts << message << endl;
    outFile.close();
}

#include <QCoreApplication>
#include <QDateTime>
#include "controller/worker.h"

#define VERSION "1.10"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
//    QCoreApplication::addLibraryPath("/usr/lib/x86_64-linux-gnu/Monita_Service");

    if (argc == 2) {
        QString arg1(argv[1]);
        if (arg1 == QLatin1String("-v") ||
            arg1 == QLatin1String("--version")) {
            QString compilationTime = QString("%1 %2").arg(__DATE__).arg(__TIME__);
            QString version = VERSION;
            printf("Monita Service Version:  %s\nPT. DaunBiru Engineering\nwww.daunbiru.com\n\n"
                   "build on: %s (UTC+7)\n",
                   version.toUtf8().data(),
                   compilationTime.toUtf8().data());
        }
        return 0;
    } else {
        printf(":: Monita Service Started ::\n");
        Worker worker;
        return a.exec();
    }
//    QObject::connect(worker, &Worker::closed, &a, &QCoreApplication::quit);
}


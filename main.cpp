#include <QCoreApplication>

#include "controller/worker.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
//    QCoreApplication::addLibraryPath("/usr/lib/x86_64-linux-gnu/Monita_Service");
    printf(":: Monita Service Started ::\n");

    Worker worker;
//    QObject::connect(worker, &Worker::closed, &a, &QCoreApplication::quit);

    return a.exec();
}


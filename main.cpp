#include <QCoreApplication>

#include "controller/worker.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    printf(":: Monita Service Started ::\n");

    Worker *worker = new Worker(1234);
//    QObject::connect(&worker, SIGNAL(finish()), &a, SLOT(quit()));
    QObject::connect(worker, &Worker::closed, &a, &QCoreApplication::quit);

    return a.exec();
}


#include <QCoreApplication>

#include "controller/worker.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    printf(":: Monita Service Started ::\n");

    Worker worker;
    QObject::connect(&worker, SIGNAL(finish()), &a, SLOT(quit()));

    return a.exec();
}


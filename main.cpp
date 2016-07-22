#include <QCoreApplication>

#include "controller/worker.h"
#include "model/monita_log.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    printf("Monita Service Started :: \n");

    Worker worker;

    return a.exec();
}


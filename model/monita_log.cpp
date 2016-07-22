#include "monita_log.h"

monita_log::monita_log()
{
    buf_lay = (char *) malloc(1024);
    buf_dateTime = (char *) malloc(32);
}

void monita_log::write(QFile *file, const char *text, ...){

    va_list args;
    int i;

    va_start (args, text);

       /* For this to work, printbuffer must be larger than
        * anything we ever want to print.
        */
    i = vsprintf (buf_lay, text, args);
    va_end (args);

    /* Print the string */
    sprintf (buf_dateTime, " [%s]\r\n", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toUtf8().data(), args);

    strcat(buf_lay, buf_dateTime);

    QTextStream printLog(file);
    printLog << buf_lay;
}

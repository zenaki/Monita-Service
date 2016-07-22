#ifndef MONITA_LOG_H
#define MONITA_LOG_H

#include "util/utama.h"


class monita_log
{
public:
    monita_log();

    char *buf_lay;
    char *buf_dateTime;

    void write(QFile *file, const char *text, ...);
};

#endif // MONITA_LOG_H

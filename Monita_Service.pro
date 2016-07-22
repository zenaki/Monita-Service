QT += core sql
QT -= gui

TARGET = Monita_Service
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

HEADERS += \
        model/get_db.h \
        controller/worker.h \
        model/save_db.h \
        model/init_mysql.h \
        util/parsing_fuction.h \
        util/utama.h \
        model/monita_log.h \
        imodbus.h \
        3rdparty/qextserialport/qextserialport.h \
        3rdparty/qextserialport/qextserialenumerator.h \
        3rdparty/libmodbus/src/modbus.h \
    util/config.h

SOURCES += main.cpp \
        model/get_db.cpp \
        controller/worker.cpp \
        model/save_db.cpp \
        model/init_mysql.cpp \
        util/parsing_fuction.cpp \
        model/monita_log.cpp \
        3rdparty/qextserialport/qextserialport.cpp \
        3rdparty/libmodbus/src/modbus.c \
        3rdparty/libmodbus/src/modbus-data.c \
        3rdparty/libmodbus/src/modbus-tcp.c \
    util/config.cpp

INCLUDEPATH += 3rdparty/libmodbus \
               3rdparty/libmodbus/src \
               3rdparty/qextserialport \
               src
unix {
    SOURCES += 3rdparty/qextserialport/posix_qextserialport.cpp	\
           3rdparty/qextserialport/qextserialenumerator_unix.cpp
    DEFINES += _TTY_POSIX_
}

win32 {
    SOURCES += 3rdparty/qextserialport/win_qextserialport.cpp \
           3rdparty/qextserialport/qextserialenumerator_win.cpp
    DEFINES += _TTY_WIN_  WINVER=0x0501
    LIBS += -lsetupapi -lws2_32
}

INCLUDEPATH += /usr/local/include/hiredis
LIBS += -L/usr/local/lib -lhiredis

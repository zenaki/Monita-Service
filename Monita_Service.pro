QT += core sql network websockets
QT -= gui

TARGET = monita-service
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

HEADERS += \
        model/get_db.h \
        controller/worker.h \
        model/save_db.h \
        model/init_mysql.h \
        util/utama.h \
        model/monita_log.h \
#        imodbus.h \
#        3rdparty/qextserialport/qextserialport.h \
#        3rdparty/qextserialport/qextserialenumerator.h \
#        3rdparty/libmodbus/src/modbus.h \
        util/config.h \
        util/redis.h \
#        util/util_skyw.h \
#        util/parsing_fuction.h \
    controller/tcp_modbus.h \
    controller/data_mysql.h \
    controller/data_visual.h \
    controller/sky_wave.h

SOURCES += main.cpp \
        model/get_db.cpp \
        controller/worker.cpp \
        model/save_db.cpp \
        model/init_mysql.cpp \
        model/monita_log.cpp \
#        3rdparty/qextserialport/qextserialport.cpp \
#        3rdparty/libmodbus/src/modbus.c \
#        3rdparty/libmodbus/src/modbus-data.c \
#        3rdparty/libmodbus/src/modbus-tcp.c \
        util/config.cpp \
        util/redis.cpp \
#        util/util_skyw.cpp \
#        util/parsing_fuction.cpp \
    controller/tcp_modbus.cpp \
    controller/data_mysql.cpp \
    controller/data_visual.cpp \
    controller/sky_wave.cpp

#INCLUDEPATH += 3rdparty/libmodbus \
#               3rdparty/libmodbus/src \
#               3rdparty/qextserialport \
#               src
#unix {
#    SOURCES += 3rdparty/qextserialport/posix_qextserialport.cpp	\
#           3rdparty/qextserialport/qextserialenumerator_unix.cpp
#    DEFINES += _TTY_POSIX_
#}

#win32 {
#    SOURCES += 3rdparty/qextserialport/win_qextserialport.cpp \
#                3rdparty/qextserialport/qextserialenumerator_win.cpp
#    DEFINES += _TTY_WIN_  WINVER=0x0501
#    LIBS += -lsetupapi -lws2_32
#}

INCLUDEPATH += 3rdparty/hiredis-64bit
LIBS += -L3rdparty/hiredis-64bit -lhiredis

INCLUDEPATH += /usr/local/include/hiredis
LIBS += -L/usr/local/lib -lhiredis

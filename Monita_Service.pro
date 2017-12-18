QT += core sql network websockets
QT -= gui

TARGET = monita-service
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

OBJECTS_DIR += compile

HEADERS +=                          \
        model/get_db.h              \
        controller/worker.h         \
        model/save_db.h             \
        model/init_mysql.h          \
        util/utama.h                \
        model/monita_log.h          \
        util/config.h               \
        util/redis.h                \
#        util/api.h                  \
        controller/data_mysql.h     \
        controller/data_visual.h    \
        controller/process.h
#    controller/http_server.h

SOURCES += main.cpp                 \
        model/get_db.cpp            \
        controller/worker.cpp       \
        model/save_db.cpp           \
        model/init_mysql.cpp        \
        model/monita_log.cpp        \
        util/config.cpp             \
        util/redis.cpp              \
#        util/api.cpp                \
        controller/data_mysql.cpp   \
        controller/data_visual.cpp  \
        controller/process.cpp
#    controller/http_server.cpp

#INCLUDEPATH += 3rdparty/hiredis-64bit
#LIBS += -L3rdparty/hiredis-64bit -lhiredis

INCLUDEPATH += /usr/local/include/hiredis
LIBS += -L/usr/local/lib -lhiredis

#message('Including core files')
#include($$PWD/../../QttpServer/core.pri)

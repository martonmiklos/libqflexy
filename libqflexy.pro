#-------------------------------------------------
#
# Project created by QtCreator 2013-07-20T12:26:43
#
#-------------------------------------------------

QT       -= gui

TARGET = libqflexy
TEMPLATE = lib

DEFINES += LIBQFLEXY_LIBRARY

SOURCES += ecr.cpp

HEADERS += ecr.h\
        libqflexy_global.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

#-------------------------------------------------
#
# Project created by QtCreator 2016-07-08T20:59:17
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = keytarboard
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    jackEngine.cpp

HEADERS  += mainwindow.h \
    jackEngine.h \
    midiDefines.h

FORMS    += mainwindow.ui


unix: CONFIG += link_pkgconfig

# Jack
unix: PKGCONFIG += jack

#-------------------------------------------------
#
# Project created by QtCreator 2014-12-08T21:00:12
#
#-------------------------------------------------

QT       += core gui sql network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = andes
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qutil.cpp \
    mythead.cpp

HEADERS  += mainwindow.h \
    qutil.h \
    mythead.h

FORMS    += mainwindow.ui

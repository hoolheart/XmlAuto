#-------------------------------------------------
#
# Project created by QtCreator 2013-06-28T18:18:38
#
#-------------------------------------------------

QT       += core
QT       += xml

QT       -= gui

TARGET = XMLauto
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    datastr.cpp \
    funs.cpp \
    ZtTable/ZtTable.cpp

HEADERS += \
    datastr.h \
    publics.h \
    funs.h \
    ZtTable/ZtTable.h

OTHER_FILES += \
    ZtTable/ZtTable.xml

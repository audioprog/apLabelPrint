#-------------------------------------------------
#
# Project created by QtCreator 2015-08-08T14:08:28
#
#-------------------------------------------------

QT       += widgets svg testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport

TARGET = tst_TestApLabelPrintTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../apqunqlite

*-g++* {
	  QMAKE_CXXFLAGS += -std=c++11
}


SOURCES += tst_TestApLabelPrintTest.cpp \
    ../apqunqlite/UnQLite/unqlite.c \
    ../apqunqlite/qunqlitecursor.cpp \
    ../apqunqlite/qunqlite.cpp \
    ../apqunqlite/apUnQLite.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    ../apqunqlite/UnQLite/unqlite.h \
    ../apqunqlite/qunqlitecursor.h \
    ../apqunqlite/dpointer.h \
    ../apqunqlite/qunqlite.h \
    ../apqunqlite/apUnQLite.h

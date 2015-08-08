#-------------------------------------------------
#
# Project created by QtCreator 2015-08-08T09:14:23
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = apLabelPrint
TEMPLATE = app

*-g++* {
	  QMAKE_CXXFLAGS += -std=c++11
}


SOURCES += main.cpp\
		apLabelPrintMainWindow.cpp \
    apqunqlite/qunqlite.cpp \
    apqunqlite/qunqlitecursor.cpp \
    apqunqlite/UnQLite/unqlite.c \
    apqunqlite/apUnQLite.cpp

HEADERS  += apLabelPrintMainWindow.h \
    apqunqlite/UnQLite/unqlite.h \
    apqunqlite/dpointer.h \
    apqunqlite/qunqlite.h \
    apqunqlite/qunqlitecursor.h \
    apqunqlite/apUnQLite.h

FORMS    += apLabelPrintMainWindow.ui

DISTFILES += \
    .gitignore \
    LICENSE.md

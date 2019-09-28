#-------------------------------------------------
#
# Project created by QtCreator 2018-11-24T21:21:49
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PostgresMonitor
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    sql.cpp \
    log.cpp \
    widgets/WidgetTransactions.cpp \
    About.cpp \
    widgets/Widget.cpp \
    widgets/WidgetSettingsServer.cpp

HEADERS  += mainwindow.h \
    sql.h \
    log.h \
    widgets/WidgetTransactions.h \
    About.h \
    widgets/Widget.h \
    widgets/WidgetSettingsServer.h

FORMS    += mainwindow.ui \
    widgets/WidgetTransactions.ui \
    About.ui \
    widgets/WidgetSettingsServer.ui

OTHER_FILES += \
    project

RESOURCES += \
    icons.qrc

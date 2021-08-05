TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

TARGET = bin/fsserver

LIBS += -lboost_system -lboost_thread -lboost_regex -lstdc++fs -lpq -lboost_filesystem

SOURCES += \
        src/main.cpp \
    src/db_manager.cpp

HEADERS += \
    include/apifss.h \
    src/db_manager.h

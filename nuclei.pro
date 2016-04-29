CONFIG += debug_and_release
UI_DIR = $$PWD/ui

TEMPLATE = app

QT += core gui network svg printsupport
#CONFIG += warn_on

QMAKE_CXXFLAGS += -DBOOST_LOG_DYN_LINK

LIBS += -lblas -lgslcblas -lgsl -lgslcblas

LIBS += -lboost_system -lboost_date_time -lboost_thread -lboost_log \
           -lboost_filesystem -lboost_log_setup -lboost_timer -lboost_regex

TARGET = $$PWD/nuclei

CONFIG -= c++11
QMAKE_CXXFLAGS += -std=c++11

DEFINES += PRINT_SEARCH_RESULTS

SOURCES += $$files($$PWD/*.cpp) \
           $$files($$PWD/NucData/*.cpp) \
           $$files($$PWD/SchemePlayer/*.cpp) \
           libakk/Akk.cpp

HEADERS  += $$files($$PWD/*.h) \
            $$files($$PWD/NucData/*.h) \
            $$files($$PWD/SchemePlayer/*.h) \
            libakk/Akk.h \
            libakk/akk_global.h

target.path = /usr/local/bin/
INSTALLS = target

INCLUDEPATH += $$PWD \
               $$PWD/NucData \
               $$PWD/SchemePlayer \
               $$PWD/libakk

FORMS += $$files($$PWD/*.ui)

RESOURCES += $$files($$PWD/resources/*.qrc)



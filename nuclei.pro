CONFIG += debug_and_release
UI_DIR = $$PWD/ui

TEMPLATE = app

BADDIRS = "debug release ui"

QMAKE_CLEAN += -r $$BADDIRS \
               $$files(qpx*.log) \
               $$files(qpx.pro.user*) \
               install

QT += core gui network svg printsupport
#CONFIG += warn_on

QMAKE_CXXFLAGS += -DBOOST_LOG_DYN_LINK

LIBS += -lboost_system -lboost_date_time -lboost_thread -lboost_log \
           -lboost_filesystem -lboost_log_setup -lboost_timer -lboost_regex

TARGET = $$PWD/nuclei

DEFINES += PRINT_SEARCH_RESULTS

unix {
  target.path = /usr/local/bin/
  icon.path = /usr/share/icons/
  desktop.path = /usr/share/applications/

  LIBPATH += /usr/local/lib

  !mac {
    CONFIG -= c++11
    QMAKE_CXXFLAGS += -std=c++11
  }

  mac {
    CONFIG += c++11
    QMAKE_LFLAGS += -lc++
    INCLUDEPATH += /usr/local/include
    QMAKE_CXXFLAGS += -stdlib=libc++ -Wno-c++11-narrowing
    QMAKE_CLEAN += $$files(*.app)
  }
}


SOURCES += $$files($$PWD/*.cpp) \
           $$files($$PWD/NucData/*.cpp) \
           $$files($$PWD/ensdf/*.cpp) \
           $$files($$PWD/SchemePlayer/*.cpp)

HEADERS  += $$files($$PWD/*.h) \
            $$files($$PWD/NucData/*.h) \
            $$files($$PWD/ensdf/*.h) \
            $$files($$PWD/SchemePlayer/*.h)
            libakk/akk_global.h

INSTALLS = target

INCLUDEPATH += $$PWD \
               $$PWD/NucData \
               $$PWD/ensdf \
               $$PWD/SchemePlayer

FORMS += $$files($$PWD/*.ui)

RESOURCES += $$files($$PWD/resources/*.qrc)



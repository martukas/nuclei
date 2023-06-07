TEMPLATE = app

QT += core gui network svg widgets printsupport x11extras
CONFIG += warn_on

mac {
    TARGET = Nuclei
} else {
    TARGET = nuclei
}

DEFINES += PRINT_SEARCH_RESULTS QT_DISABLE_DEPRECATED_BEFORE=0x000000

SOURCES += main.cpp\
    Decay.cpp \
    EnergyLevel.cpp \
    Nuclide.cpp \
    HalfLife.cpp \
    SpinParity.cpp \
    GammaTransition.cpp \
    ActiveGraphicsItemGroup.cpp \
    GraphicsHighlightItem.cpp \
    GraphicsDropShadowEffect.cpp \
    ClickableItem.cpp \
    ENSDFDownloader.cpp \
    SearchDialog.cpp \
    HalfLifeSpinBox.cpp \
    Energy.cpp \
    ENSDFDataSource.cpp \
    DecayCascadeItemModel.cpp \
    AbstractDataSource.cpp \
    DecayCascadeFilterProxyModel.cpp \
    LineEdit.cpp \
    ENSDFParser.cpp \
    SearchResultDataSource.cpp \
    SearchConstraints.cpp \
    TreeView.cpp \
    ProxyStyle.cpp \
    Nuclei.cpp \
    UpdateCheck.cpp \
    UncertainDouble.cpp

HEADERS  += \
    Decay.h \
    EnergyLevel.h \
    Nuclide.h \
    HalfLife.h \
    SpinParity.h \
    GammaTransition.h \
    ActiveGraphicsItemGroup.h \
    GraphicsHighlightItem.h \
    GraphicsDropShadowEffect.h \
    ClickableItem.h \
    version.h \
    ENSDFDownloader.h \
    SearchDialog.h \
    HalfLifeSpinBox.h \
    Energy.h \
    ENSDFDataSource.h \
    AbstractDataSource.h \
    DecayCascadeItemModel.h \
    DecayCascadeFilterProxyModel.h \
    LineEdit.h \
    ENSDFParser.h \
    SearchResultDataSource.h \
    SearchConstraints.h \
    TreeView.h \
    ProxyStyle.h \
    Nuclei.h \
    UpdateCheck.h \
    UncertainDouble.h

isEmpty(PREFIX) {
    PREFIX=/usr/local
}
target.path = $$PREFIX/bin/
INSTALLS = target

INCLUDEPATH += $$PREFIX/include ../libakk ../qtftp/include /usr/local/Qxt/include /usr/local/qwt-6.1.5/include
mac:LIBS += -lakk -L$$PREFIX/lib -L../libakk
unix:LIBS += -L../libakk -lakk -L/usr/local/qwt-6.1.5/lib -lqwt -L../qtftp/lib -lQt5Ftp -L/usr/local/Qxt/lib -lQxtWidgets
#unix:release:LIBS += -L../libakk
#unix:debug:LIBS += -L../libakk

!win32:LIBS += -lquazip

FORMS    += \
    ENSDFDownloader.ui \
    ENSDFDownloaderSettings.ui \
    PreferencesDialog.ui \
    SearchDialog.ui \
    Nuclei.ui \
    UpdateCheckDialog.ui

RESOURCES += \
    nuclei.qrc

RC_FILE = nuclei.rc

mac {
    ICON = nuclei.icns
} else {
    CONFIG += qxt
    QXT += core gui
}

macx-clang {
    message(Optimizing for OSX Clang)
    QMAKE_CXXFLAGS_RELEASE += "-O4"
    QMAKE_LFLAGS_RELEASE += "-O4"
}

# enable building of universal binary on old macs
macx-g++ {
    QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.5.sdk
    CONFIG += ppc x86
}


win32-msvc* {
    CONFIG += static exceptions
    LIBS += ../libakk/release/akk.lib
    INCLUDEPATH += C:/boost_1_52_0 ../quazip
    LIBS += ../qwt/lib/qwt.lib ../quazip/quazip/release/quazip.lib
    INCLUDEPATH += ../qwt/src
    LIBS += ../gsl-1.15-msvc/_build-msvc/libcblas/libcblas.lib ../gsl-1.15-msvc/_build-msvc/libgsl/libgsl.lib
    DEFINES += QUAZIP_STATIC
    DEFINES += QXT_STATIC
    DEFINES += BOOST_ALL_NO_LIB
    #QMAKE_LFLAGS_RELEASE += /VERBOSE:LIB
    QMAKE_LFLAGS_CONSOLE    = /SUBSYSTEM:CONSOLE,5.01
    QMAKE_LFLAGS_WINDOWS    = /SUBSYSTEM:WINDOWS,5.01
}


# QWT
CONFIG += qwt



QT += core gui network svg
CONFIG += warn_on

CONFIG += qxt
QXT += core gui

TARGET = nuclei
TEMPLATE = app


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
    Nuclei.cpp

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
    Nuclei.h

INCLUDEPATH += ../../libakk/src
LIBS += -lakk -L../../libakk
LIBS += -lquazip

FORMS    += \
    ENSDFDownloader.ui \
    ENSDFDownloaderSettings.ui \
    PreferencesDialog.ui \
    SearchDialog.ui \
    Nuclei.ui

RESOURCES += \
    nuclei.qrc

RC_FILE = nuclei.rc

# QWT ####################

CONFIG += qwt

exists( /usr/include/qwt/qwt.h ) {
  INCLUDEPATH += /usr/include/qwt
  LIBS += -lqwt
}

exists( /usr/include/qwt6/qwt.h ) {
  INCLUDEPATH += /usr/include/qwt6
  LIBS += -lqwt6
}

exists( /usr/include/qwt-qt4/qwt.h ) {
  INCLUDEPATH += /usr/include/qwt-qt4
  LIBS += -lqwt-qt4
}

exists( C://build//qwt//src//qwt.h ) {
  INCLUDEPATH += C://build//qwt//src
  LIBS += -LC://build//qwt//lib -lqwt
}


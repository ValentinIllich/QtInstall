# -------------------------------------------------
# Project created by QtCreator 2009-07-23T11:26:59
# -------------------------------------------------
TARGET = QtInstall
TEMPLATE = app

SOURCES += main.cpp \
    wizard.cpp \
    welcomepage.cpp \
    datacabinet.cpp \
    datagramfilehandler.cpp \
    datagramsettingshandler.cpp \
    installpage.cpp \
    pathmanagement.cpp \
    datagramlinkshandler.cpp
HEADERS += wizard.h \
    welcomepage.h \
    datacabinet.h \
    datagramfilehandler.h \
    datagramsettingshandler.h \
    installpage.h \
    pathmanagement.h \
    datagramlinkshandler.h
RESOURCES += QtInstall.qrc
OTHER_FILES += definition.csv \
    welcome.html
FORMS += 

MOC_DIR = ./tmp
UI_DIR = ./tmp
OBJECTS_DIR = ./tmp
RCC_DIR = ./tmp

CONFIG -= exceptions

win32 {
   RC_FILE      = ressources/QtInstall.rc
}
mac {
    ICON = ressources/QtInstall.icns
    QMAKE_INFO_PLIST = ressources/Info_mac.plist
}

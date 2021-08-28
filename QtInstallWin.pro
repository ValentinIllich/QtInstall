# -------------------------------------------------
# Project created by QtCreator 2009-07-23T11:26:59
# -------------------------------------------------
TARGET = QtInstall

TEMPLATE = app

QT += core widgets gui

SOURCES += main.cpp \
    wizard.cpp \
    welcomepage.cpp \
    datacabinet.cpp \
    datagramfilehandler.cpp \
    datagramsettingshandler.cpp \
    installpage.cpp \
    pathmanagement.cpp \
    datagramlinkshandler.cpp \
    licensepage.cpp \
    completepage.cpp \
    ../backup/utilities.cpp \
    authexec.c
HEADERS += wizard.h \
    welcomepage.h \
    datacabinet.h \
    datagramfilehandler.h \
    datagramsettingshandler.h \
    installpage.h \
    pathmanagement.h \
    datagramlinkshandler.h \
    licensepage.h \
    completepage.h \
    ../backup/utilities.h

RESOURCES += QtInstall.qrc
OTHER_FILES += definition.csv \
    welcome.html \
    ressources/readme.txt \
    ressources/license.txt \
    ressources/QtInstall.rc \
    ressources/Info_mac.plist \
    LICENSE.html \
    ReleaseNotes.txt

win32 {
    RC_FILE = ressources/QtInstall.rc
    LIBS += -static-libgcc
}
macx {
    ICON = ressources/QtInstall.icns
    QMAKE_INFO_PLIST = ressources/Info_mac.plist
    LIBS += -framework Security
}
CONFIG += exceptions

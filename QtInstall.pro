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
    datagramlinkshandler.cpp \
    licensepage.cpp \
    completepage.cpp \
    ../utilities.cpp
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
    ../utilities.h
RESOURCES += QtInstall.qrc
OTHER_FILES += definition.csv \
    welcome.html \
    ressources/readme.txt \
    ressources/license.txt \
    LICENSE.html \
    ReleaseNotes.txt
FORMS += 
win32 { 
    MOC_DIR = c:/tmp/qtinstall_obj
    UI_DIR = c:/tmp/qtinstall_obj
    OBJECTS_DIR = c:/tmp/qtinstall_obj
    RCC_DIR = c:/tmp/qtinstall_obj
    RC_FILE = ressources/QtInstall.rc
}
macx { 
    QMAKE_MAKEFILE=MacMakefile
    MOC_DIR = /var/tmp/qtinstall_obj
    UI_DIR = /var/tmp/qtinstall_obj
    OBJECTS_DIR = /var/tmp/qtinstall_obj
    RCC_DIR = /var/tmp/qtinstall_obj
    ICON = ressources/QtInstall.icns
    QMAKE_INFO_PLIST = ressources/Info_mac.plist
}
CONFIG -= exceptions

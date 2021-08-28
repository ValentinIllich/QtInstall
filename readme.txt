QtInstall README (C) Valentin Illich 2009/2010/2013

QtInstall Mac got an important bug fix

- crash fixed on systems with CPU prior to Dual Core 2 processors

QtInstall Win/Mac got an update

- supporting execution with admin rights (using UAC in Win 7 & Mac OS) if needed 
- correct tracking of installed shared files

QtInstall has now a stable state.

- fully supporting for Mac Os X and Windows XP / 7
- setup creation wizard and preview ability

- not uninstalling files which were existing during installation

coming up in the future:

- extended wizard for creating setup definitions
- setup components

Please refer to ReleaseNotes.txt



################

Static installation of QT 5.12

- download src archive 
- cd in source directory


Windows:
========

configure -prefix %CD%\qtbase -release -static -static-runtime -accessibility -no-icu -no-sql-sqlite -no-qml-debug -no-opengl -nomake examples -nomake tests

nmake (or 'mingw32-make') module-qtbase module-qtdeclarative module-qttools module-qttranslations module-qtwinextras



Mac: (QT 5.12.11)
=================

./configure -prefix $PWD/qtbase -release -static -no-securetransport -accessibility -qt-zlib -qt-libpng -qt-libjpeg -no-cups -no-sql-sqlite -no-qml-debug -nomake examples -nomake tests -no-freetype

make module-qtbase module-qtdeclarative module-qttools module-qttranslations
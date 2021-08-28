rm -fR QtInstall.app/Contents/Frameworks
mkdir QtInstall.app/Contents/Frameworks
cp -R /Users/Shared/QT-4.8.4-x86/lib/QtCore.stripped.framework QtInstall.app/Contents/Frameworks/QtCore.framework
cp -R /Users/Shared/QT-4.8.4-x86/lib/QtGui.stripped.framework QtInstall.app/Contents/Frameworks/QtGui.framework

install_name_tool -id @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore QtInstall.app/Contents/Frameworks/QtCore.framework/Versions/4/QtCore
install_name_tool -id @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui QtInstall.app/Contents/Frameworks/QtGui.framework/Versions/4/QtGui

install_name_tool -change /Users/Shared/QT-4.8.4-x86/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore QtInstall.app/Contents/MacOs/QtInstall
install_name_tool -change /Users/Shared/QT-4.8.4-x86/lib/QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui QtInstall.app/Contents/MacOs/QtInstall

install_name_tool -change /Users/Shared/QT-4.8.4-x86/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore QtInstall.app/Contents/Frameworks/QtGui.framework/Versions/4/QtGui
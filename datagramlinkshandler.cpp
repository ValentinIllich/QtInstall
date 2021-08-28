#include "datagramlinkshandler.h"
#include "pathmanagement.h"
#include "datacabinet.h" // dbgout()

#include <QDir>
#include <QProcess>
#include <QSettings>

#include <QmessageBox>

#define MACDESKTOP      "<HOMEDIR>/Desktop"

void DatagramLinksHandler::processLink(linkCommand cmd,QString const &properties,QString const &target,QString const &iconfile,int attributes)
{
    QString destination = PathManagement::replaceSymbolicNames(target);
	QFileInfo info(destination);
    QString appname = QDir::tempPath() + "/helperapp";
    QString linkPath;
    QStringList args;
    bool useHelperApp = false;

    dbgout(QString("--- processing link '")+destination+"' with helper '"+appname+"'");

    QProcess *proc = new QProcess(0);
    QObject::connect(proc, SIGNAL(finished(int)), proc, SLOT(deleteLater()));

    switch( cmd )
    {
        case eCreateStartupLink:
            dbgout("    ...creating Startup Link");
            useHelperApp = true;
            #if defined(Q_OS_WIN32)
            {
				QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",QSettings::NativeFormat);
                linkPath = settings.value("Startup").toString();
                args.append("-createLink");
                args.append(destination);
                args.append(linkPath+"/"+info.baseName()+".lnk"); // linkfile
                args.append(info.dir().path()+"/"+iconfile); // icon
                args.append(info.dir().path()); // working dir
            }
            #endif
            #if defined(Q_OS_MAC)
                args.append("-createLink");
                args.append(destination);
            #endif
            break;
       case eRemoveStartupLink:
            dbgout("    ...remove Startup Link");
            useHelperApp = true;
            #if defined(Q_OS_WIN32)
            {
				QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",QSettings::NativeFormat);
                linkPath = settings.value("Startup").toString();
                QFileInfo info(destination);
                args.append("-removeLink");
                args.append(linkPath+"/"+info.baseName()+".lnk"); // linkfile
            }
            #endif
            #if defined(Q_OS_MAC)
                args.append("-removeLink");
                args.append(destination);
            #endif
            break;
        case eCreateDesktopLink:
            dbgout("    ...creating Desktop Link");
            #if defined(Q_OS_WIN32)
            {
                useHelperApp = true;
				QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",QSettings::NativeFormat);
                linkPath = settings.value("Desktop").toString();
                QFileInfo info(destination);
                args.append("-createLink");
                args.append(destination);
                args.append(linkPath+"/"+info.baseName()+".lnk"); // linkfile
                args.append(info.dir().path()+"/"+iconfile); // icon
                args.append(info.dir().path()); // working dir
            }
            #endif
            #if defined(Q_OS_MAC)
            {
                linkPath = PathManagement::replaceSymbolicNames(MACDESKTOP);
                QFileInfo info(destination);
                appname = "/bin/ln";
                args.append("-s");
                args.append("-f");
                args.append(destination);
                args.append(linkPath+"/"+info.fileName());
            }
            #endif
            break;
        case eRemoveDesktopLink:
            dbgout("    ...remove Desktop Link");
            #if defined(Q_OS_WIN32)
            {
                useHelperApp = true;
				QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",QSettings::NativeFormat);
                linkPath = settings.value("Desktop").toString();
                QFileInfo info(destination);
                args.append("-removeLink");
                args.append(linkPath+"/"+info.baseName()+".lnk"); // linkfile
            }
            #endif
            #if defined(Q_OS_MAC)
            {
                linkPath = PathManagement::replaceSymbolicNames(MACDESKTOP);
                QFileInfo info(destination);
                appname = "/bin/rm";
                args.append(linkPath+"/"+info.fileName());
            }
            #endif
            break;
    }

    if( useHelperApp )
    {
        #if defined(Q_OS_WIN32)
            appname = QDir::tempPath() + "/helperapp.exe";
            if( !QFile::copy(":/helperApps/InstallDriver.exe",appname) )
                dbgout("### cannot create helper application!");
        #endif
        #if defined(Q_OS_MAC)
            appname = QDir::tempPath() + "/helperapp";
            if( !QFile::copy(":/helperApps/SSLoginItems.bin",appname) )
                dbgout("### cannot create helper application!");
        #endif
        if( !QFile::setPermissions(appname,	QFile::ReadOwner|QFile::ReadUser|QFile::ReadGroup|QFile::ReadOther|
                                                QFile::WriteOwner|QFile::WriteOwner|QFile::WriteOwner|QFile::WriteOwner|
                                                QFile::ExeOwner|QFile::ExeUser|QFile::ExeGroup|QFile::ExeOther) )
            dbgout("### permissions error on helper application!");
    }

    proc->start(appname, args);
    proc->waitForFinished();
    dbgout(QString("... code, stat=")+QString::number(proc->exitCode())+"/"+QString::number(proc->exitStatus()));
    //Mac: code!=0 Fehler!

    if( proc->exitCode()!=0 )
        dbgout("### helper application returned error!");

    delete proc;

    if( useHelperApp )
    {
        if( !QFile::remove(appname) )
            dbgout(QString("### cannot delete application ")+appname);
    }
}

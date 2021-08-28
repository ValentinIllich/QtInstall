#include "datagramlinkshandler.h"
#include "pathmanagement.h"
#include "datacabinet.h" // dbgout()

#include <QDir>
#include <QProcess>
#include <QSettings>

#include <QmessageBox>

#include "../utilities.h"

#define MACDESKTOP      "<HOMEDIR>/Desktop"

bool DatagramLinksHandler::processLink(linkCommand cmd,QString const &properties,QString const &target,QString const &iconfile,int attributes)
{
    QString destination = PathManagement::replaceSymbolicNames(target);
    QString icon = PathManagement::replaceSymbolicNames(iconfile);
	QFileInfo info(destination);
    QString appname = QDir::tempPath() + "/helperapp";
    QString linkPath;
    QStringList args;
    bool useHelperApp = false;
    bool error = false;

    dbgout(QString("--- processing link '")+destination+"' with helper '"+appname+"'");

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
                args.append(/*info.dir().path()+"/"+*/icon); // icon
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
                args.append(/*info.dir().path()+"/"+*/icon); // icon
                args.append(info.dir().path()); // working dir
            }
            #endif
            #if defined(Q_OS_MAC)
            {
                linkPath = PathManagement::replaceSymbolicNames(MACDESKTOP);
                QFileInfo info(destination);
                if( QFile::exists(linkPath+"/"+info.fileName()) )
                    appname ="";
                else
                {
                    appname = "/bin/ln";
                    args.append("-s");
                    args.append("-f");
                    args.append(destination);
                    args.append(linkPath+"/"+info.fileName());
                }
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
                if( QFile::exists(linkPath+"/"+info.fileName()) )
                {
                    appname = "/bin/rm";
                    args.append(linkPath+"/"+info.fileName());
                }
                else
                    appname = "";
            }
            #endif
            break;
    }

    if( useHelperApp )
    {
        #if defined(Q_OS_WIN32)
            appname = QDir::tempPath() + "/helperapp.exe";
            if( !QFile::copy(":/helperApps/InstallHelper.exe",appname) )
                dbgout("### cannot create helper application!");
        #endif
        #if defined(Q_OS_MAC)
            appname = QDir::tempPath() + "/helperapp";
            if( !QFile::copy(":/helperApps/SSLoginItems.bin",appname) )
                error = dbgout("### cannot create helper application!");
        #endif
        if( !QFile::setPermissions(appname,	QFile::ReadOwner|QFile::ReadUser|QFile::ReadGroup|QFile::ReadOther|
                                                QFile::WriteOwner|QFile::WriteOwner|QFile::WriteOwner|QFile::WriteOwner|
                                                QFile::ExeOwner|QFile::ExeUser|QFile::ExeGroup|QFile::ExeOther) )
           error =  dbgout("### permissions error on helper application!");
    }

    if( !appname.isEmpty() )
    {
        QProcess *proc = new QProcess(0);
        QObject::connect(proc, SIGNAL(finished(int)), proc, SLOT(deleteLater()));

        proc->start(appname, args);
        proc->waitForFinished();
        dbgout(QString("... code, stat=")+QString::number(proc->exitCode())+"/"+QString::number(proc->exitStatus()));
        //Mac: code!=0 Fehler!

        if( proc->exitCode()!=0 )
            error = dbgout("### helper application returned error!");

        delete proc;

        if( useHelperApp )
        {
            if( !QFile::remove(appname) )
                error = dbgout(QString("### cannot delete helper application ")+appname);
        }
    }
    else
        dbgout("...nothing to do.");

    return error;
}

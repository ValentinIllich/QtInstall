#include "datagramlinkshandler.h"
#include "pathmanagement.h"
#include "datacabinet.h" // dbgout()

#include <QDir>
#include <QProcess>
#include <QSettings>

#include <QMessageBox>

#include "../backup/Utilities.h" // dbgout()

#define MACDESKTOP      "<HOMEDIR>/Desktop"

static bool m_debugMode = false;

bool DatagramLinksHandler::processLink(linkCommand cmd,QString const &properties,QString const &target,QString const &iconfile,int attributes)
{
    QString destination = PathManagement::replaceSymbolicNames(target);
    QString icon = PathManagement::replaceSymbolicNames(iconfile);
    QString macHelper = "";
    QString appname = QDir::tempPath() + "/helperapp";
    QString linkPath;
    QStringList args;
    bool useHelperApp = false;
    bool error = false;

    QString linkName;
    if( destination.endsWith(".app") )
      linkName = QDir(destination).dirName().remove(".app");
    else
      linkName = QFileInfo(destination).fileName();

    switch( cmd )
    {
        case eCreateStartupLink:
            dbgout("    ...creating Startup Link",1);
            useHelperApp = true;
            #if defined(Q_OS_WIN32)
            {
                QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",QSettings::NativeFormat);
                linkPath = settings.value("Startup").toString();
                args.append("-createLink");
                args.append(destination);
                args.append(linkPath+"/"+linkName+".lnk"); // linkfile
                args.append(/*info.dir().path()+"/"+*/icon); // icon
                args.append(QFileInfo(destination).dir().path()); // working dir
            }
            #endif
            #if defined(Q_OS_MAC)
                macHelper = "makestartup.sh";
                args.append(linkName); // backup
                args.append(destination); // /Applications/VISolutions/Automatic Backup/backup.app
            #endif
            break;
       case eRemoveStartupLink:
            dbgout("    ...remove Startup Link",1);
            useHelperApp = true;
            #if defined(Q_OS_WIN32)
            {
                QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",QSettings::NativeFormat);
                linkPath = settings.value("Startup").toString();
                QFileInfo info(destination);
                args.append("-removeLink");
                args.append(linkPath+"/"+linkName+".lnk"); // linkfile
            }
            #endif
            #if defined(Q_OS_MAC)
                macHelper = "removestartup.sh";
                args.append(linkName); // backup
            #endif
            break;
        case eCreateDesktopLink:
            dbgout("    ...creating Desktop Link",1);
            useHelperApp = true;
            #if defined(Q_OS_WIN32)
            {
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
                macHelper = "makealias.sh";
                args.append(linkName); // backup
                args.append(destination);
            }
            #endif
            break;
        case eRemoveDesktopLink:
            dbgout("    ...remove Desktop Link",1);
            useHelperApp = true;
            #if defined(Q_OS_WIN32)
            {
                QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",QSettings::NativeFormat);
                linkPath = settings.value("Desktop").toString();
                QFileInfo info(destination);
                args.append("-removeLink");
                args.append(linkPath+"/"+info.baseName()+".lnk"); // linkfile
            }
            #endif
            #if defined(Q_OS_MAC)
            {
                macHelper = "removealias.sh";
                args.append(linkName); // backup
            }
            #endif
            break;
    }

    if( properties!="uninstalling" ) createUndo(cmd,target);

    if( useHelperApp )
    {
        #if defined(Q_OS_WIN32)
            appname = QDir::tempPath() + "/helperapp.exe";
            if( !QFile::copy(":/helperApps/InstallHelper.exe",appname) )
                dbgout("### cannot create helper application!",0);
            dbgout(QString("--- processing link '")+destination+"' with link name '"+linkName+"' and helper 'InstallHelper.exe'",2);
        #endif
        #if defined(Q_OS_MAC)
            appname = QDir::tempPath() + "/helperapp.sh";
            if( !QFile::exists(":/helperApps/"+macHelper) )
              error = dbgout("### cannot find helper application "+macHelper,0);
            if( !QFile::copy(":/helperApps/"+macHelper,appname) )
                error = dbgout("### cannot create helper application!",0);
            dbgout(QString("--- processing link '")+destination+"' with link name '"+linkName+"' and helper '"+ macHelper +"'",2);
        #endif
        if( !QFile::setPermissions(appname,	QFile::ReadOwner|QFile::ReadUser|QFile::ReadGroup|QFile::ReadOther|
                                                QFile::WriteOwner|QFile::WriteOwner|QFile::WriteOwner|QFile::WriteOwner|
                                                QFile::ExeOwner|QFile::ExeUser|QFile::ExeGroup|QFile::ExeOther) )
           error =  dbgout("### permissions error on helper application!",0);
    }

    if( !appname.isEmpty() )
    {
        if( m_debugMode )
        {
            dbgout("calling "+appname+" "+args.join(" "),0);
        }
        else
        {
            QProcess *proc = new QProcess(0);
            QObject::connect(proc, SIGNAL(finished(int)), proc, SLOT(deleteLater()));

            proc->start(appname, args);
            proc->waitForFinished();
            dbgout(QString("... code, stat=")+QString::number(proc->exitCode())+"/"+QString::number(proc->exitStatus()),2);
            //Mac: code!=0 Fehler!

            if( proc->exitCode()!=0 )
            {
                error = dbgout("### helper application returned error!",0);
                dbgout(proc->readAllStandardOutput(),0);
                dbgout(proc->readAllStandardError(),0);
            }

            delete proc;
        }

        if( useHelperApp )
        {
            if( !QFile::remove(appname) )
                error = dbgout(QString("### cannot delete helper application ")+appname,0);
        }
    }
    else
        dbgout("...nothing to do.",2);

    return error;
}

bool DatagramLinksHandler::createUndo(linkCommand cmd,QString const &target)
{
    QFile file(DataCabinet::getUninstallFile());
    if( file.open(QFile::ReadOnly) )
    {
        QByteArray fileData = file.readAll();
        file.close();

        if( file.open(QFile::WriteOnly | QFile::Truncate) )
        {
            QString uninstallCommand = "";

            switch( cmd )
            {
                case eCreateStartupLink:
                    uninstallCommand = "sl:"+PathManagement::replaceSymbolicNames(target) +"\r\n";
                break;
                case eCreateDesktopLink:
                    uninstallCommand = "dl:"+PathManagement::replaceSymbolicNames(target) +"\r\n";
                break;
            default:
              break;
            }
            file.write(uninstallCommand.toLatin1(),uninstallCommand.length());
            file.write(fileData);
            file.close();
        }
    }

    return true;
}

void DatagramLinksHandler::setDebugMode(bool debugging)
{
    m_debugMode = debugging;
}

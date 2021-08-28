#include "datagramsettingshandler.h"
#include "pathmanagement.h"
#include "datacabinet.h"

#include <QSettings>
#include <QCoreApplication>

#include "../backup/Utilities.h" // dbgout()

static bool m_debugMode = false;

bool DatagramSettingsHandler::processSetting(QString const &key,QString const &properties,int attributes,QString const &value)
{
    if( attributes==settingsAppAndOrgName )
    {
        if( properties!="uninstalling" ) createUndo(key,attributes,value);

        if( m_debugMode )
            return false;

        dbgout(QString("--- setting organization '")+key+"' and aplication '"+value+"'",1);

        QCoreApplication::setOrganizationName(key);
        QCoreApplication::setApplicationName(value);
    }
    else
    {
        dbgout(QString("--- processing setting '")+key+"'",2);

        QString setting = PathManagement::replaceSymbolicNames(key);
        bool specialKey = false;

        QString myKey = PathManagement::replaceSymbolicNames(key);
        QString myValue = PathManagement::replaceSymbolicNames(value);

        if( setting.startsWith("<WINCURRENTVERS>") )
        {
            setting = setting.replace("<WINCURRENTVERS>","HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion");
            int lastsep = setting.lastIndexOf('\\');
            if( lastsep>= 0 )
            {
                myKey = setting.mid(lastsep+1);
                setting = setting.left(lastsep);
            }
            specialKey = true;
        }

        if( properties!="uninstalling" ) createUndo(myKey,attributes,"");

        if( m_debugMode )
            return false;

        if( specialKey )
        {
            setting.replace("/","\\");
            myValue.replace("/","\\");
            QSettings sett(setting,QSettings::NativeFormat);
            if( attributes==settingsRemoveSettings )
                sett.remove(myKey);
            else
                sett.setValue(myKey,myValue);
        }
        else
        {
            QSettings sett;
            if( attributes==settingsRemoveSettings )
                sett.remove(myKey);
            else
                sett.setValue(myKey,myValue);
        }
    }

    return false;
}

bool DatagramSettingsHandler::createUndo(QString const &key,int attributes,QString const &value)
{
    QFile file(DataCabinet::getUninstallFile());
    if( file.open(QFile::WriteOnly | QFile::Append) )
    {
        QString uninstallCommand = "";
        if( attributes==settingsAppAndOrgName )
            uninstallCommand = "sa:"+key + ";" + value +"\r\n";
        else
            uninstallCommand = "rs:"+PathManagement::replaceSymbolicNames(key) +"\r\n";
        file.write(uninstallCommand.toLatin1(),uninstallCommand.length());
        file.close();
    }

    return true;
}

void DatagramSettingsHandler::setDebugMode(bool debugging)
{
    m_debugMode = debugging;
}

#include "datagramsettingshandler.h"
#include "pathmanagement.h"
#include "datacabinet.h"

#include <QSettings>
#include <QCoreApplication>

#include "../utilities.h" // dbgout()

bool DatagramSettingsHandler::processSetting(QString const &key,QString const &properties,int attributes,QString const &value)
{
    if( attributes==settingsAppAndOrgName )
    {
        dbgout(QString("--- setting organization '")+key+"' and aplication '"+value+"'");

        QCoreApplication::setOrganizationName(key);
        QCoreApplication::setApplicationName(value);
    }
    else
    {
        dbgout(QString("--- processing setting '")+key+"'");

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

        if( specialKey )
        {
            setting.replace("/","\\");
            myValue.replace("/","\\");
            QSettings sett(setting,QSettings::NativeFormat);
            sett.setValue(myKey,myValue);
        }
        else
        {
            QSettings sett;
            sett.setValue(myKey,myValue);
        }
    }

    return false;
}

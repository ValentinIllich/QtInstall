#include "datagramfilehandler.h"
#include "datacabinet.h" // dbgout()
#include "pathmanagement.h"

#include <QFileInfo>
#include <QStringList>
#include <QDir>

QString DatagramFileHandler::m_applicationPath = "/Applications";

int DatagramFileHandler::getPropertiesFromString(QString const &str)
{
    int ret = 0;

    if( str.contains("copySrc") )
        ret |= useFilePermissions;
    if( str.contains("exec") )
        ret |= executablePermission;
//    if( str.contains("") )
//        ret |= ;

    return ret;
}

/******************************************************/

void DatagramFileHandler::processFile(QString const &properties,QString const &destination,QDateTime const &lastModified,int attributes,int filePermissions,QByteArray const &data)
{
    dbgout(QString("--- processing file '")+destination+"'");

    QString filename = destination;
    bool copyIt = false;

    filename = PathManagement::replaceSymbolicNames(filename);

    if( attributes==removeDestination )
    {
        copyIt = false;
        QFile::remove(filename);
    }
    else
    {
        QFileInfo fi(filename);
        if( fi.exists() )
        {
            if( lastModified>fi.lastModified() )
                copyIt = true;
        }
        else
        {
            QStringList pathtree = filename.split("/");
            QString actualPath = pathtree.at(0); // the first given directory must exist here!
            for( int i=1; i<pathtree.count()-1; i++ )
            {
                QString testDir = actualPath+"/"+pathtree.at(i);

                dbgout(QString("    trying dir '")+testDir+"'...");

                QDir dir(testDir);
                if( !dir.exists() )
                {
                    dbgout("    ... not existant, creating it...");
                    if( !QDir(actualPath).mkdir(pathtree.at(i)) )
                        dbgout(QString("### can't create directory ")+actualPath+"/"+pathtree.at(i)); // error!
                }
                actualPath = testDir;
            }

            copyIt = true;
        }
    }

    if( copyIt )
    {
        dbgout(QString("    --> now copying data of file '")+filename+"'.");
        QFile file(filename);
        if( file.open(QIODevice::WriteOnly) )
        {
            if( attributes & useFilePermissions )
            {
                file.setPermissions((QFile::Permissions)filePermissions);
            }
            if( attributes & executablePermission )
            {
                QFile::Permissions actual = file.permissions();
                file.setPermissions(actual | QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther);
            }
            file.write(data);
            file.close();
        }
        else
           dbgout(QString("### can't open destintion file!")+filename); //error!
    }
    else
        dbgout("    --> nothing to do.");
}

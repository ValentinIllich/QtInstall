#include "datagramfilehandler.h"
#include "datacabinet.h"
#include "pathmanagement.h"

#include <QFileInfo>
#include <QStringList>
#include <QDir>

#include "../backup/Utilities.h" // dbgout()

QString DatagramFileHandler::m_applicationPath = "/Applications";
static bool m_debugMode = false;

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

QString packetRegistration(int sharedRegistrationCommand,QString const &destination,QString const &packageID)
{
  installSettings sett;

  QString searchKey = destination;
  searchKey.replace("/",":");

  QString packetUsingThis = sett.value(searchKey,"");
  QString retval;

  switch(sharedRegistrationCommand)
  {
  case checkPackageID:
    retval = packetUsingThis;
    break;
  case insertPackageID:
    retval = packetUsingThis;
    if( !packetUsingThis.contains(packageID) )
    {
      packetUsingThis = packetUsingThis.append(packageID+";");
      sett.setValue(searchKey,packetUsingThis);
    }
    break;
  case removePackageID:
    if( packetUsingThis.contains(packageID) )
    {
      packetUsingThis = packetUsingThis.remove(packageID+";");
      if( packetUsingThis.isEmpty() )
        sett.remove(searchKey);
      else
        sett.setValue(searchKey,packetUsingThis);
    }
    retval = packetUsingThis;
    break;
  }

  return retval;
}

bool DatagramFileHandler::processFile(QString const &packageID,QString const &properties,QString const &destination,QDateTime const &lastModified,int attributes,int filePermissions,QByteArray const &data)
{
  dbgout(QString("--- processing file '")+destination+"'",2);

  QString filename = destination;
  bool copyIt = false;
  bool error = false;
  bool needsAdmin = false;

  filename = PathManagement::replaceSymbolicNames(filename,&needsAdmin);

  if( attributes==removeDestination )
  {
    copyIt = false;
    QFile::remove(filename);
    packetRegistration(removePackageID,filename,packageID);
  }
  else
  {
    createUndo(destination);

    QFileInfo fi(filename);
    if( fi.exists() )
    {
      QString registeredPackages = packetRegistration(checkPackageID,filename,packageID);

      if( registeredPackages.isEmpty() )
        // fiel does not come from QtInstall
        dbgout(QString("    --> not touching, file already exists: '")+filename+"', file size is "+QString::number(fi.size()),1);
      else
      {
        packetRegistration(insertPackageID,filename,packageID);
        if( lastModified>fi.lastModified() )
          copyIt = true;
      }
    }
    else
    {
      packetRegistration(insertPackageID,filename,packageID);

      bool skip = false;
      if( needsAdmin && !PathManagement::hasAdminAcces() )
        skip = true;

      if( skip )
        error = dbgout("### copying of file '"+filename+"' requires admin rights!",0);
      else
      {
        if( m_debugMode )
          return false;

        QStringList pathtree = filename.split("/");
        QString actualPath = pathtree.at(0); // the first given directory must exist here!
        for( int i=1; i<pathtree.count()-1; i++ )
        {
          QString testDir = actualPath+"/"+pathtree.at(i);

          dbgout(QString("    trying dir '")+testDir+"'...",2);

          QDir dir(testDir);
          if( !dir.exists() )
          {
            dbgout("    ... not existant, creating it...",2);
            if( !QDir(actualPath).mkdir(pathtree.at(i)) )
              error = dbgout(QString("### can't create directory ")+actualPath+"/"+pathtree.at(i),0); // error!
          }
          actualPath = testDir;
        }

        copyIt = true;
      }
    }
  }

  if( copyIt )
  {
    QByteArray uncompressed = qUncompress(data);
    dbgout(QString("    --> now copying data of file '")+filename+"', file size is "+QString::number(uncompressed.size()),2);
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
      file.write(uncompressed);
      file.close();
    }
    else
      error = dbgout(QString("### can't open destintion file!")+filename,0); //error!
  }
  else
    dbgout("    --> nothing to do.",2);

  return error;
}

bool DatagramFileHandler::createUndo(QString const &destination)
{
  QFile file(DataCabinet::getUninstallFile());
  if( file.open(QFile::WriteOnly | QFile::Append) )
  {
    QString uninstallCommand = "rf:"+PathManagement::replaceSymbolicNames(destination) +"\r\n";
    file.write(uninstallCommand.toLatin1(),uninstallCommand.length());
    file.close();
  }

  return true;
}

bool DatagramFileHandler::processUndo(QString const &packageID,QString const &destination)
{
  bool error = false;

  if( !packetRegistration(checkPackageID,destination,packageID).contains(packageID) )
  {
    dbgout(QString("    --> file did exist upon installation: not removing '")+destination+"'",1);
    return false;
  }

  if( !packetRegistration(removePackageID,destination,packageID).isEmpty() )
  {
    dbgout(QString("    --> file in use by other packages: not removing '")+destination+"'",1);
    return false;
  }

  dbgout(QString("    --> file may be removed: '")+destination+"'",2);

  if( m_debugMode )
    return false;

  QFile file(destination);
  if( file.remove() )
  {
    QFileInfo info(destination);
    QDir dir = info.dir();

    QStringList list(dir.entryList());
    while( list.count()<=2 )
    {
      QFile::remove(dir.absolutePath()+"/.DS_Store");
      QString name = dir.dirName();
      dir.cdUp();
      if( !dir.rmdir(name) )
        QMessageBox::warning(0,"dir error","directory\n"+dir.absolutePath()+"/"+name+"\nseems to have (hidden) content.");
      else
        list = dir.entryList();
    }
  }
  else
    error = dbgout(QString("### can't remove destintion file!")+destination,0); //error!

  return error;
}

void DatagramFileHandler::setDebugMode(bool debugging)
{
  m_debugMode = debugging;
}

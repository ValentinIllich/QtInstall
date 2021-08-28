#include "datacabinet.h"
#include "datagramfilehandler.h"
#include "datagramsettingshandler.h"
#include "datagramlinkshandler.h"
#include "pathmanagement.h"

#include <QFileInfo>
#include <QDateTime>
#include <QPlainTextEdit>
#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QDir>

#include "../utilities.h"

QString DataCabinet::m_emptyProperty = "";
QString DataCabinet::m_uninstallFile = "";

DataCabinet::DataCabinet()
  : m_iLastElement(0)
  , m_iElementCount(0)
  , m_iCabinetVersion(0)
//  , m_iElementScanned(0)
  , m_progressHandler(NULL)
  , m_error(false)
  , m_debugMode(false)
{
  setDebugMode(false);
  m_properties << "" << "" << "" << "" << "" << "" << "" << "";
  //    setProperty(0,"This is a QtInstall cabinet data file");
}

void DataCabinet::setProperty(PropertyID ID,QString const &value)
{
  while( ID>=m_properties.count() )
    m_properties.append("");
  m_properties[ID] = value;
}
QString const &DataCabinet::getProperty(PropertyID ID)
{
  if( ID<m_properties.count() )
    return m_properties.at(ID);
  else
    return m_emptyProperty;
}

bool DataCabinet::openFile(QString const &filename,int fileOffset)
{
  m_iLastElement = -1;

  m_file.setFileName(filename);
  if( m_file.open(QIODevice::ReadOnly) )
  {
    if( fileOffset>0 )
      m_file.seek(fileOffset);

    cabinetMagic myCabinetMagic;

    m_file.read((char*)&myCabinetMagic,sizeof(myCabinetMagic));

    bool ret = true;

    if( ret && myCabinetMagic.magic!='VISL' )
    {
      error("### this .qip file seems not to be a QtInstall package (magic wrong)!");
      ret = false;
    }

    if( ret && myCabinetMagic.version!=1001001 )
    {
      error("### this .qip file seems not to be a QtInstall package (version wrong)!");
      ret = false;
    }

    if( ret )
    {
      QString properties = m_file.read(myCabinetMagic.descrLength);
      m_properties = properties.split("@propval@");

      m_iElementCount = myCabinetMagic.nelements;
      m_iCabinetVersion = myCabinetMagic.version;
      dbgout(QString("\n**** opened cabinet file version '")+QString::number(myCabinetMagic.version)+"' with "+QString::number(myCabinetMagic.nelements)+" elemnts in it",1);

      dbgout(QString("Window  Title: ")+m_properties.at(ePropWindowTitle),1,true);
      dbgout(QString("Welcome  Text: ")+m_properties.at(ePropWelcomeText),1,true);
      dbgout(QString("Complete Text: ")+m_properties.at(ePropCompletionText),1,true);
      dbgout(QString("License  Text: ")+m_properties.at(ePropLicenceText),1,true);
      dbgout(QString("  Setup Comp.: ")+m_properties.at(ePropComponentDefinition),1,true);

      QString installPath = PathManagement::replaceSymbolicNames("<APPDATADIR>");
      QDir dir(installPath);

      if( !dir.exists("QtInstall") )
        dir.mkdir("QtInstall");

      m_uninstallFile = installPath +"/QtInstall/" + getProperty(ePropSetupId) + ".csv";

      //            bool isMajor,isMinor;
      //            if( !isUpdateInstallation(isMajor,isMinor) )
      //                ret = false;
    }
    else
      m_file.close();

    return ret;
  }
  else
  {
    error("### cannot open this .qip file!");
    return false;
  }
}

bool DataCabinet::createFile(QString const &filename)
{
  m_iLastElement = 0;

  m_file.setFileName(filename);
  if( m_file.open(QIODevice::ReadWrite | QIODevice::Truncate) )
  {
    cabinetMagic myCabinetMagic;
    QString description = m_properties.join("@propval@");
    myCabinetMagic.magic = 'VISL';
    myCabinetMagic.version = 1001001;
    myCabinetMagic.nelements = 0;
    myCabinetMagic.descrLength = description.length();

    m_file.write((char*)&myCabinetMagic,sizeof(myCabinetMagic));
    m_file.write(description.toLatin1(),myCabinetMagic.descrLength);

    dbgout(QString("\n**** created cabinet file version '")+QString::number(myCabinetMagic.version)+"'",2);

    return true;
  }
  else
  {
    error("### could not create the QtInstall package!");
    return false;
  }
}

void DataCabinet::closeFile()
{
  if( m_iLastElement>=0 )
  {
    // this was writing the file!
    cabinetMagic myCabinetMagic;

    m_file.seek(0);
    m_file.read((char*)&myCabinetMagic,sizeof(myCabinetMagic));

    myCabinetMagic.nelements = m_iLastElement;

    m_file.seek(0);
    m_file.write((char*)&myCabinetMagic,sizeof(myCabinetMagic));
  }
  m_file.close();
}

void DataCabinet::setProgressHandler(IDataProgress *handler)
{
  m_progressHandler = handler;
}

bool DataCabinet::scanFile()
{
  m_error = false;

  int actualElement = 0;

  //    QFile::remove(m_uninstallFile);
  QFile file(DataCabinet::getUninstallFile());
  if( file.open(QFile::WriteOnly | QFile::Truncate) )
  {
    file.close();
  }

  if( m_progressHandler ) m_progressHandler->setProgress(0);

  while( actualElement<m_iElementCount )
  {
    cabinetHeader myCabinetHeader;

    m_file.read((char*)&myCabinetHeader,sizeof(myCabinetHeader));
    switch( myCabinetHeader.ID )
    {
            case dataFileDatagram:
      dbgout("+++ found dataFileDatagram",2);
      m_error = m_error | scanFileDatagram(m_file,myCabinetHeader.attributes);
      break;
            case settingsDatagram:
      dbgout("+++ found dataSettingsDatagram",2);
      m_error = m_error | scanSettingsDatagram(m_file,myCabinetHeader.attributes);
      break;
           case linksDatagram:
      dbgout("+++ found linksDatagram",2);
      m_error = m_error | scanLinksDatagram(m_file,myCabinetHeader.attributes);
      break;
            default:
      m_file.seek(m_file.pos()+myCabinetHeader.dataLength);
      break;
    }

    actualElement++;
    if( m_progressHandler ) m_progressHandler->setProgress(100*actualElement/m_iElementCount);
  }

  if( m_progressHandler ) m_progressHandler->setProgress(100);
  dbgout("--- finished parsing.",2);

  registerInstallation();

  return true;
}

bool DataCabinet::scanFileDatagram(QFile &fileptr,int attributes)
{
  fileDataHeader myFileDataHeader;

  fileptr.read((char*)&myFileDataHeader,sizeof(myFileDataHeader));

  int filenameLength = myFileDataHeader.destinationLength;
  QString destination = fileptr.read(filenameLength);
  int propertiesLength = myFileDataHeader.propertiesLength;
  QString properties = fileptr.read(propertiesLength);

  QDateTime modified;
  modified.setTime_t(myFileDataHeader.lastModified);

  int size = myFileDataHeader.dataLength;
  QByteArray data;

  if( size>0 )
    data = fileptr.read(size);

  dbgout(QString("    destination='")+destination+"', lastModified='"+modified.toString()+"', attributes='"+QString::number(attributes)+"', compressed size="+QString::number(size)+" bytes",1);

  return DatagramFileHandler::processFile(getProperty(ePropSetupId),properties,destination,modified,attributes,myFileDataHeader.filePermissions,data);
}
bool DataCabinet::scanSettingsDatagram(QFile &fileptr,int attributes)
{
  settingsDataHeader mySettingsDataHeader;

  fileptr.read((char*)&mySettingsDataHeader,sizeof(mySettingsDataHeader));

  int keyLength = mySettingsDataHeader.keyLength;
  QString key = fileptr.read(keyLength);
  int propertiesLength = mySettingsDataHeader.propertiesLength;
  QString properties = fileptr.read(propertiesLength);

  int valueLength = mySettingsDataHeader.valueLength;
  QString value = fileptr.read(valueLength);;

  dbgout(QString("    key='")+key+"', value='"+value+"', attributes='"+QString::number(attributes)+"'",1);

  return DatagramSettingsHandler::processSetting(key,properties,attributes,value);
}
bool DataCabinet::scanLinksDatagram(QFile &fileptr,int attributes)
{
  linksDataHeader myLinksDataHeader;

  fileptr.read((char*)&myLinksDataHeader,sizeof(myLinksDataHeader));

  int targetLength = myLinksDataHeader.targetLength;
  QString target = fileptr.read(targetLength);
  int propertiesLength = myLinksDataHeader.propertiesLength;
  QString properties = fileptr.read(propertiesLength);
  int iconFileLength = myLinksDataHeader.iconFileLength;
  QString iconFile = fileptr.read(iconFileLength);

  return DatagramLinksHandler::processLink((DatagramLinksHandler::linkCommand)myLinksDataHeader.operation,properties,target,iconFile,attributes);
}

bool DataCabinet::hasError()
{
  return m_error;
}
void DataCabinet::setDebugMode(bool debugging)
{
  m_debugMode = debugging;
  DatagramFileHandler::setDebugMode(debugging);
  DatagramSettingsHandler::setDebugMode(debugging);
  DatagramLinksHandler::setDebugMode(debugging);
}

void DataCabinet::appendFileDatagram(QString const &srcFilename,QString const &dstFilename,QString const &properties,int attributes)
{
  cabinetHeader myCabinetHeader;
  fileDataHeader myFileDataHeader;
  QByteArray compressed;

  QFile file(srcFilename);
  if( file.open(QIODevice::ReadOnly) || attributes==removeDestination )
  {
    if( attributes==removeDestination )
    {
      myFileDataHeader.destinationLength = dstFilename.length();
      myFileDataHeader.dataLength = 0;
      myFileDataHeader.propertiesLength = properties.length();
      myFileDataHeader.lastModified = 0;
      myFileDataHeader.filePermissions = 0;
    }
    else
    {
      QFileInfo info(srcFilename);

      compressed = qCompress(file.readAll());

      myFileDataHeader.destinationLength = dstFilename.length();
      myFileDataHeader.dataLength = compressed.size();
      myFileDataHeader.propertiesLength = properties.length();
      myFileDataHeader.lastModified = info.lastModified().toTime_t();
      myFileDataHeader.filePermissions = file.permissions();
    }

    myCabinetHeader.ID = dataFileDatagram;
    myCabinetHeader.attributes = attributes;
    myCabinetHeader.dataLength = sizeof(myFileDataHeader) + myFileDataHeader.destinationLength + myFileDataHeader.propertiesLength + myFileDataHeader.dataLength;

    m_file.write((const char*)&myCabinetHeader,sizeof(myCabinetHeader));
    m_file.write((const char*)&myFileDataHeader,sizeof(myFileDataHeader));
    m_file.write(dstFilename.toLatin1(),myFileDataHeader.destinationLength);
    m_file.write(properties.toLatin1(),myFileDataHeader.propertiesLength);

    if( attributes!=removeDestination )
    {
      m_file.write(compressed,myFileDataHeader.dataLength);
      file.close();
    }

    dbgout(QString("+++ appendFileDatagram with destination '")+dstFilename+"', original size "+QString::number(file.size())+", size "+
           QString::number(myFileDataHeader.dataLength)+" bytes, attributes "+QString::number(attributes),1);

    m_iLastElement++;
  }
  else
    error(QString("### source file '")+srcFilename+"' not found!");
}

void DataCabinet::appendSettingsDatagram(QString const &key,QString const &value,QString const &properties,int attributes)
{
  cabinetHeader myCabinetHeader;
  settingsDataHeader mySettingsDataHeader;

  mySettingsDataHeader.keyLength = key.length();
  mySettingsDataHeader.propertiesLength = properties.length();
  mySettingsDataHeader.valueLength = value.length();

  myCabinetHeader.ID = settingsDatagram;
  myCabinetHeader.attributes = attributes;
  myCabinetHeader.dataLength = sizeof(mySettingsDataHeader) + mySettingsDataHeader.keyLength + mySettingsDataHeader.propertiesLength + mySettingsDataHeader.valueLength;

  m_file.write((const char*)&myCabinetHeader,sizeof(myCabinetHeader));
  m_file.write((const char*)&mySettingsDataHeader,sizeof(mySettingsDataHeader));
  m_file.write(key.toLatin1(),mySettingsDataHeader.keyLength);
  m_file.write(properties.toLatin1(),mySettingsDataHeader.propertiesLength);
  m_file.write(value.toLatin1(),mySettingsDataHeader.valueLength);

  dbgout(QString("+++ appendSettingsDatagram with key '")+key+"', value '"+
         value+"', attributes "+QString::number(attributes),1);

  m_iLastElement++;
}

void DataCabinet::appendLinksDatagram(int linkCommand,QString const &target,QString const &iconfile,QString const &properties,int attributes)
{
  cabinetHeader myCabinetHeader;
  linksDataHeader myLinksDataHeader;

  myLinksDataHeader.targetLength = target.length();
  myLinksDataHeader.iconFileLength = iconfile.length();
  myLinksDataHeader.propertiesLength = properties.length();
  myLinksDataHeader.operation = linkCommand;

  myCabinetHeader.ID = linksDatagram;
  myCabinetHeader.attributes = attributes;
  myCabinetHeader.dataLength = sizeof(myLinksDataHeader) + myLinksDataHeader.targetLength + myLinksDataHeader.propertiesLength + myLinksDataHeader.iconFileLength;

  m_file.write((const char*)&myCabinetHeader,sizeof(myCabinetHeader));
  m_file.write((const char*)&myLinksDataHeader,sizeof(myLinksDataHeader));
  m_file.write(target.toLatin1(),myLinksDataHeader.targetLength);
  m_file.write(properties.toLatin1(),myLinksDataHeader.propertiesLength);
  m_file.write(iconfile.toLatin1(),myLinksDataHeader.iconFileLength);

  dbgout(QString("+++ appendLinksDatagram with target '")+target+"', icon '"+
         iconfile+"', attributes "+QString::number(attributes),1);

  m_iLastElement++;
}

void DataCabinet::registerInstallation()
{
  QCoreApplication::setOrganizationName("VISolutions.de");
  QCoreApplication::setApplicationName("QtInstall");

  QSettings sett;

  QString versionEntry = getProperty(ePropSetupMajor) + ";" + getProperty(ePropSetupMinor);
  sett.setValue(getProperty(ePropSetupId),versionEntry);
}

bool DataCabinet::isUpdateInstallation(bool &isMajor,bool &isMinor)
{
  QSettings sett;

  QString key = getProperty(ePropSetupId);
  QString versionEntry = sett.value(key,"").toString();

  if( versionEntry.isEmpty() )
  {
    isMajor = false;
    isMinor = true;
  }
  else
  {
    QStringList specs = versionEntry.split(";");

    isMajor = specs.at(0)!=getProperty(ePropSetupMajor);
    isMinor = specs.at(1)!=getProperty(ePropSetupMinor);
  }

  return isMajor || isMinor;
}

void DataCabinet::undoInstallation()
{
  m_error = false;

  csvFile file(DataCabinet::getUninstallFile());
  if( file.open(QIODevice::ReadOnly) )
  {
    m_iElementCount = 0;
    while( file.bytesAvailable() )
    {
      QString line = file.readCsvLine();
      m_iElementCount++;
    }
    file.seek(0);

    if( m_progressHandler ) m_progressHandler->setProgress(0);

    int actualElement = 0;
    while( file.bytesAvailable() )
    {
      // read entry
      QString input = file.readCsvLine();

      if( input.left(3)=="rf:" )
      {
        dbgout(QString("    remove file destination='")+input.mid(3)+"'",1);
        m_error = m_error | DatagramFileHandler::processUndo(getProperty(ePropSetupId),input.mid(3));
      }
      if( input.left(3)=="sa:" )
      {
        dbgout(QString("    settings defaults='")+input.mid(3)+"'",1);
        QStringList list = input.mid(3).split(";");
        m_error = m_error | DatagramSettingsHandler::processSetting(list.at(0),"uninstalling",settingsAppAndOrgName,list.at(1));
      }
      if( input.left(3)=="rs:" )
      {
        dbgout(QString("    remove settings='")+input.mid(3)+"'",1);
        m_error = m_error | DatagramSettingsHandler::processSetting(input.mid(3),"uninstalling",settingsRemoveSettings,"");
      }
      if( input.left(3)=="sl:" )
      {
        dbgout(QString("    remove startup link='")+input.mid(3)+"'",1);
        m_error = m_error | DatagramLinksHandler::processLink(DatagramLinksHandler::eRemoveStartupLink,"uninstalling",input.mid(3),"",0);
      }
      if( input.left(3)=="dl:" )
      {
        dbgout(QString("    remove desktop link='")+input.mid(3)+"'",1);
        m_error = m_error | DatagramLinksHandler::processLink(DatagramLinksHandler::eRemoveDesktopLink,"uninstalling",input.mid(3),"",0);
      }

      if( m_progressHandler ) m_progressHandler->setProgress(100*actualElement/m_iElementCount);

      actualElement++;
    }

    file.close();

    QCoreApplication::setOrganizationName("VISolutions.de");
    QCoreApplication::setApplicationName("QtInstall");

    QSettings sett;
    sett.remove(getProperty(ePropSetupId));
    //sett.setValue(getProperty(ePropSetupId),"");
  }
}

QString DataCabinet::getUninstallFile()
{
  return m_uninstallFile;
}

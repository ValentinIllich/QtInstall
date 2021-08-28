#include "pathmanagement.h"

#include <QDataStream>

#define APPDIR      "<APPDIR>"
#define SYSDIR      "<SYSDIR>"
#define HOMEDIR     "<HOMEDIR>"
#define USERNAME    "<USER>"
#define ADDPATADIR  "<APPDATADIR>"

#define ADMINTESTFILEWINXP  "<SYSDIR>/test.dll"
#define ADMINTESTFILEWIN7   "<HOMEDIR>/AppData/Local/VirtualStore/Windows/test.dll"
#define ADMINTESTFILEMAC    "<SYSDIR>/test.dylib"

QStringList PathManagement::symbolicNames;

QString PathManagement::m_applicationsDir = "";
QString PathManagement::m_systemDir = "";

QString PathManagement::m_HomeDir = "";
QString PathManagement::m_User = "";
QString PathManagement::m_applicationsData = "";

// QString m_applicationsDir = "";
// QString m_applicationsDir = "";
// QString m_applicationsDir = "";

bool PathManagement::m_hasAccess = false;

QString getUnifiedEnvPath(const char *envVar)
{
    char *ptr = getenv(envVar);
    QString temp;

    if( ptr )
        temp = ptr;
    else
        temp = "";

    temp.replace("\\","/");

    return temp;
}

void PathManagement::init()
{
    symbolicNames << APPDIR << SYSDIR << HOMEDIR << USERNAME << ADDPATADIR;

#ifdef Q_OS_WIN32
    m_applicationsDir = getUnifiedEnvPath("ProgramFiles");
    m_systemDir = getUnifiedEnvPath("SystemRoot");

    m_HomeDir = getUnifiedEnvPath("USERPROFILE");
    m_User = getUnifiedEnvPath("USERNAME");
    m_applicationsData = getUnifiedEnvPath("APPDATA");

    QString testFile = PathManagement::replaceSymbolicNames(ADMINTESTFILEWINXP);
#endif

#ifdef Q_OS_DARWIN
    m_applicationsDir = "/Applications";
    m_systemDir = "/Library/Caches";

    m_HomeDir = getUnifiedEnvPath("HOME");
    m_User = getUnifiedEnvPath("USER");
    m_applicationsData = m_HomeDir + "/Library/Application Support";

    QString testFile = PathManagement::replaceSymbolicNames(ADMINTESTFILEMAC);
#endif

    QFile adminTest(testFile);
    if( adminTest.open(QIODevice::WriteOnly) )
    {
        // file could be created, then user should have admin rights under Win XP
        adminTest.close();
        m_hasAccess = true;

        QString testFile2 = PathManagement::replaceSymbolicNames(ADMINTESTFILEWIN7);
        if( QFile::exists(testFile2) ) // testing for the file in Windows 7 Virtual Store
        {
            // file found in store, so user should not have admin rights under Win 7
            m_hasAccess = false;
        }
    }
    QFile::remove(testFile);
}

QString PathManagement::getAppDir()
{
    return m_applicationsDir;
}
QString PathManagement::getSystemDir()
{
    return m_systemDir;
}

QString PathManagement::getUser()
{
  return m_User;
}

QString PathManagement::getHomeDir()
{
    return m_HomeDir;
}
QString PathManagement::getAppDataDir()
{
    return m_applicationsData;
}

QString PathManagement::replaceSymbolicNames(QString const &path,bool *needsAdminAccess)
{
    bool adminNeeded = false;
    QString result = path;

    if( result.contains(APPDIR) || result.contains(SYSDIR) )
        adminNeeded = true;

    result.replace(APPDIR,m_applicationsDir);
    result.replace(SYSDIR,m_systemDir);
    result.replace(HOMEDIR,m_HomeDir);
    result.replace(USERNAME,m_User);
    result.replace(ADDPATADIR,m_applicationsData);

    if( needsAdminAccess ) *needsAdminAccess = adminNeeded;
    return result;
}
QStringList PathManagement::getSymbolicPathNames()
{
    return symbolicNames;
}

bool PathManagement::hasAdminAcces()
{
    return m_hasAccess;
}


installSettings::installSettings()
: m_changed(false)
{
  QString filename = PathManagement::replaceSymbolicNames("<SYSDIR>/de.VISolutions.QtInstall");
  QFile settingsfile(filename);
  if( QFile::exists(filename) )
  {
    settingsfile.open(QIODevice::ReadOnly);
    QDataStream str(&settingsfile);
    str >> m_settings;
    settingsfile.close();
  }
  QString logname = PathManagement::replaceSymbolicNames("<SYSDIR>/de.VISolutions.QtInstall.log");
  if( FILE *fp=fopen(logname.toLatin1().data(),"w") )
  {
    QMap<QString, QString>::const_iterator i = m_settings.constBegin();
    while (i != m_settings.constEnd())
    {
     fprintf(fp,"%s=%s\n",i.key().toLatin1().data(),i.value().toLatin1().data());
       ++i;
    }
    fclose(fp);
  }
}

installSettings::~installSettings()
{
  if( m_changed )
  {
    QString filename = PathManagement::replaceSymbolicNames("<SYSDIR>/de.VISolutions.QtInstall");
    QFile settingsfile(filename);
    if( settingsfile.open(QIODevice::WriteOnly | QIODevice::Truncate) )
    {
      QDataStream str(&settingsfile);
      str << m_settings;
      settingsfile.close();
    }
  }
}

bool installSettings::exists(QString const &key)
{
  return m_settings.contains(key);
}

QString installSettings::value(QString const &key, QString const &defaultValue)
{
  if( m_settings.contains(key) )
    return m_settings[key];
  else
    return defaultValue;
}

void installSettings::setValue(QString const &key, QString const &value)
{
  m_settings[key] = value;
  m_changed = true;
}

void installSettings::remove(QString const &key)
{
  m_settings.remove(key);
  m_changed = true;
}

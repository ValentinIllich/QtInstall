#include "pathmanagement.h"

#define APPDIR      "<APPDIR>"
#define SYSDIR      "<SYSDIR>"
#define HOMEDIR     "<HOMEDIR>"
#define ADDPATADIR  "<APPDATADIR>"

#define ADMINTESTFILEWINXP  "<SYSDIR>/test.dll"
#define ADMINTESTFILEWIN7   "<HOMEDIR>/AppData/Local/VirtualStore/Windows/test.dll"
#define ADMINTESTFILEMAC    "<SYSDIR>/test.dylib"

QStringList PathManagement::symbolicNames;

QString PathManagement::m_applicationsDir = "";
QString PathManagement::m_systemDir = "";

QString PathManagement::m_HomeDir = "";
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
    symbolicNames << APPDIR << SYSDIR << HOMEDIR << ADDPATADIR;

#ifdef Q_OS_WIN32
    m_applicationsDir = getUnifiedEnvPath("ProgramFiles");
    m_systemDir = getUnifiedEnvPath("SystemRoot");

    m_HomeDir = getUnifiedEnvPath("USERPROFILE");
    m_applicationsData = getUnifiedEnvPath("APPDATA");

    QString testFile = PathManagement::replaceSymbolicNames(ADMINTESTFILEWINXP);
#endif

#ifdef Q_OS_DARWIN
    m_applicationsDir = "/Applications";
    m_systemDir = "/Library";

    m_HomeDir = getUnifiedEnvPath("HOME");
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

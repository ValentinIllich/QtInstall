#include "pathmanagement.h"

#define APPDIR      "<APPDIR>"
#define SYSDIR      "<SYSDIR>"
#define HOMEDIR     "<HOMEDIR>"
#define ADDPATADIR  "<APPDATADIR>"

QStringList PathManagement::symbolicNames;

QString PathManagement::m_applicationsDir = "";
QString PathManagement::m_systemDir = "";

QString PathManagement::m_HomeDir = "";
QString PathManagement::m_applicationsData = "";

// QString m_applicationsDir = "";
// QString m_applicationsDir = "";
// QString m_applicationsDir = "";

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
#endif

#ifdef Q_OS_DARWIN
    m_applicationsDir = "/Applications";
    m_systemDir = "/Library";

    m_HomeDir = getUnifiedEnvPath("HOME");
    m_applicationsData = m_HomeDir + "/Library/Application Support";
#endif
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

QString PathManagement::replaceSymbolicNames(QString const &path)
{
    QString result = path;

    result.replace(APPDIR,m_applicationsDir);
    result.replace(SYSDIR,m_systemDir);
    result.replace(HOMEDIR,m_HomeDir);
    result.replace(ADDPATADIR,m_applicationsData);

    return result;
}
QStringList PathManagement::getSymbolicPathNames()
{
    return symbolicNames;
}

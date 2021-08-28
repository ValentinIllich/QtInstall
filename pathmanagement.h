#ifndef PATHMANAGEMENT_H
#define PATHMANAGEMENT_H

#include <QString>
#include <QFile>
#include <QStringList>

class PathManagement
{
public:
    static void init();

    static QString getAppDir();
    static QString getSystemDir();

    static QString getHomeDir();
    static QString getAppDataDir();

    static QString replaceSymbolicNames(QString const &path);
    static QStringList getSymbolicPathNames();

private:
    static QStringList symbolicNames;

    static QString m_applicationsDir;
    static QString m_systemDir;
    static QString m_HomeDir;
    static QString m_applicationsData;
};

#endif // PATHMANAGEMENT_H

#ifndef PATHMANAGEMENT_H
#define PATHMANAGEMENT_H

#include <QString>
#include <QFile>
#include <QStringList>
#include <QMap>

class PathManagement
{
public:
    static void init();

    static QString getAppDir();
    static QString getSystemDir();

    static QString getUser();

    static QString getHomeDir();
    static QString getAppDataDir();

    static QString replaceSymbolicNames(QString const &path,bool *needsAdminAccess = NULL);
    static QStringList getSymbolicPathNames();

    static bool hasAdminAcces();

private:
    static QStringList symbolicNames;

    static QString m_applicationsDir;
    static QString m_systemDir;
    static QString m_HomeDir;
    static QString m_User;
    static QString m_applicationsData;

    static bool m_hasAccess;
};

class installSettings
{
public:
  installSettings();
  virtual ~installSettings();

  bool exists(QString const &key);
  QString value(QString const &key, QString const &defaultValue);
  void setValue(QString const &key, QString const &value);
  void remove(QString const &key);

private:
  QMap<QString,QString> m_settings;
  bool m_changed;
};

#endif // PATHMANAGEMENT_H

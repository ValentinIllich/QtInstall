#ifndef DATAGRAMFILEHANDLER_H
#define DATAGRAMFILEHANDLER_H

#include <QString>
#include <QDateTime>

class DatagramFileHandler
{
public:
    static int getPropertiesFromString(QString const &str);
    static void processFile(QString const &properties,QString const &destination,QDateTime const &lastModified,int attributes,int filePermissions,QByteArray const &data);

private:
    static QString m_applicationPath;
};

#endif // DATAGRAMFILEHANDLER_H

#ifndef DATAGRAMFILEHANDLER_H
#define DATAGRAMFILEHANDLER_H

#include <QString>
#include <QDateTime>

class DatagramFileHandler
{
public:
    static int getPropertiesFromString(QString const &str);
    static bool processFile(QString const &properties,QString const &destination,QDateTime const &lastModified,int attributes,int filePermissions,QByteArray const &data);
    static bool createUndo(QString const &destination);

	static bool processUndo(QString const &destination);

    static void setDebugMode(bool debugging);

private:
    static QString m_applicationPath;
};

#endif // DATAGRAMFILEHANDLER_H

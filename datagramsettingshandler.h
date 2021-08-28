#ifndef DATAGRAMSETTINGSHANDLER_H
#define DATAGRAMSETTINGSHANDLER_H

#include <QString>
#include <QDateTime>

class DatagramSettingsHandler
{
public:
    static bool processSetting(QString const &key,QString const &properties,int attributes,QString const &value);
    static bool createUndo(QString const &key,int attributes,QString const &value);

    static void setDebugMode(bool debugging);
};

#endif // DATAGRAMSETTINGSHANDLER_H

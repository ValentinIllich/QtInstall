#ifndef DATAGRAMSETTINGSHANDLER_H
#define DATAGRAMSETTINGSHANDLER_H

#include <QString>
#include <QDateTime>

class DatagramSettingsHandler
{
public:
    static bool processSetting(QString const &key,QString const &properties,int attributes,QString const &value);
};

#endif // DATAGRAMSETTINGSHANDLER_H

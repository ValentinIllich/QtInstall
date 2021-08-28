#ifndef DATAGRAMLINKSHANDLER_H
#define DATAGRAMLINKSHANDLER_H

#include <QString>

class DatagramLinksHandler
{
public:
    enum linkCommand
    {
        eCreateStartupLink,
        eRemoveStartupLink,
        eCreateDesktopLink,
        eRemoveDesktopLink
    };

    static void processLink(linkCommand cmd,QString const &properties,QString const &target,QString const &iconfile,int attributes);
};

#endif // DATAGRAMLINKSHANDLER_H

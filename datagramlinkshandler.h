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

    static bool processLink(linkCommand cmd,QString const &properties,QString const &target,QString const &iconfile,int attributes);
    static bool createUndo(linkCommand cmd,QString const &target);

    static void setDebugMode(bool debugging);
};

#endif // DATAGRAMLINKSHANDLER_H

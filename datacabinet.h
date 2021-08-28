#ifndef DATACABINET_H
#define DATACABINET_H

#include <QString>
#include <QStringList>
#include <QFile>

extern bool dbgVisible(bool showWindow = false);
extern void dbgout(QString const &text);

#define useFilePermissions      0x00000001      // attributes for fileDatagrams
#define executablePermission    0x00000002
#define removeDestination       0x00000004

#define settingsAppAndOrgName   0x00000001      // attributes for settingsDatagrams

enum cabinetDatagramID
{
    undefinedDatagram   = -1,

    dataFileDatagram,
    settingsDatagram,
    linksDatagram
};

struct cabinetMagic                 //  + 4 byte magic + 4 byte version + 4 byte number of elements + 4 byte description length <n> + ... n bytes description ... +
{
    unsigned int magic;
    unsigned int version;
    unsigned int nelements;
    unsigned int descrLength;
};
struct cabinetHeader                //  + 4 byte ID + 4 byte attributes + 4 byte data length <n> + .... n bytes data .... +
{
    cabinetDatagramID ID;
    unsigned int attributes;
    unsigned int dataLength;
};
struct fileDataHeader               //  + 4 byte filename length <l> + 4 bytes data length <n> + 4 bytes properties length <m> +
{                                   // 4 bytes modification date + 4 bytes permissions + ...l bytes filename... + ...m bytes properties... + ...n bytes file data... +
    unsigned int destinationLength;
    unsigned int dataLength;
    unsigned int propertiesLength;
    unsigned int lastModified;
    unsigned int filePermissions;
};
struct settingsDataHeader           //  + 4 byte key length <l> + 4 bytes value length <n> + 4 bytes properties length <m> +
{                                   // ...l bytes keyame... + ...m bytes properties... + ...n bytes value... +
    unsigned int keyLength;
    unsigned int valueLength;
    unsigned int propertiesLength;
};
struct linksDataHeader               //  + 4 byte target length <l> + 4 bytes iconfile length <n> + 4 bytes operation +
{                                    // ...l bytes target... + ...n bytes iconfile... +
    unsigned int targetLength;
    unsigned int iconFileLength;
    unsigned int propertiesLength;
    unsigned int operation;
};

enum PropertyID
{
    ePropWindowTitle,
    ePropWelcomeText,
    ePropCompletionText,
    ePropLicenceText,
    ePropComponentDefinition,
};

class IDataProgress
{
public:
    virtual ~IDataProgress() {}
    virtual void setProgress(int progress) = 0;
};

class DataCabinet
{
public:
    DataCabinet();

    void setProperty(PropertyID ID,QString const &value);
    QString const &getProperty(PropertyID ID);

    bool openFile(QString const &filename);
    bool createFile(QString const &filename);
    void closeFile();

    void setProgressHandler(IDataProgress *handler);
    bool scanFile();

    void appendFileDatagram(QString const &srcFilename,QString const &dstFilename,QString const &properties,int attributes);
    void appendSettingsDatagram(QString const &key,QString const &value,QString const &properties,int attributes);
    void appendLinksDatagram(int linkCommand,QString const &target,QString const &iconfile,QString const &properties,int attributes);

    void scanFileDatagram(QFile &fileptr,int attributes);
    void scanSettingsDatagram(QFile &fileptr,int attributes);
    void scanLinksDatagram(QFile &fileptr,int attributes);

    bool hasError();

private:
    QStringList m_properties;

    QFile       m_file;
    int         m_iLastElement;

    int         m_iElementCount;
    int         m_iCabinetVersion;
    int         m_iElementScanned;

    IDataProgress   *m_progressHandler;

    bool        m_error;
};

#endif // DATACABINET_H

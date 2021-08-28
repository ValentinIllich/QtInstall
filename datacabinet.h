/*

QtInstall - the ultimative platform independent QT installer tool
Copyright (C) 2009 Valentin Illich

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

*/
#ifndef DATACABINET_H
#define DATACABINET_H

#include <QString>
#include <QStringList>
#include <QFile>

#define QtInstallVersion        "1.6"

#define useFilePermissions      0x00000001      // attributes for fileDatagrams
#define executablePermission    0x00000002
#define removeDestination       0x00000004

#define settingsAppAndOrgName   0x00000001      // attributes for settingsDatagrams
#define settingsRemoveSettings  0x00000002

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
    ePropSetupId,
    ePropSetupMajor,
    ePropSetupMinor,
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

    bool openFile(QString const &filename,int fileOffset = -1);
    bool createFile(QString const &filename);
    void closeFile();

    void setProgressHandler(IDataProgress *handler);
    bool scanFile();

    void appendFileDatagram(QString const &srcFilename,QString const &dstFilename,QString const &properties,int attributes);
    void appendSettingsDatagram(QString const &key,QString const &value,QString const &properties,int attributes);
    void appendLinksDatagram(int linkCommand,QString const &target,QString const &iconfile,QString const &properties,int attributes);

    bool scanFileDatagram(QFile &fileptr,int attributes);
    bool scanSettingsDatagram(QFile &fileptr,int attributes);
    bool scanLinksDatagram(QFile &fileptr,int attributes);

    void registerInstallation();
    bool isUpdateInstallation(bool &isMajor,bool &isMinor);
    void undoInstallation();

    bool hasError();
	void setDebugMode(bool debugging);

    static QString getUninstallFile();

private:
    QStringList m_properties;

    QFile       m_file;
    int         m_iLastElement;

    int         m_iElementCount;
    int         m_iCabinetVersion;
    int         m_iElementScanned;

    IDataProgress   *m_progressHandler;

    static QString m_emptyProperty;
    static QString m_uninstallFile;

    bool        m_error;
	bool		m_debugMode;
};

class csvFile : public QFile
{
public:
	csvFile(QString const &filename);

	QString readCsvLine();

private:
	qint64 readCsvLineData( char * data, qint64 maxSize, bool &EOL );
};

#endif // DATACABINET_H

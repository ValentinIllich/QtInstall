#include <QApplication>
#include <QDir>
#include <QFileInfoList>
#include <QMessageBox>
#include <QFileDialog>

#include "wizard.h"
#include "datacabinet.h"
#include "datagramfilehandler.h"
#include "datagramlinkshandler.h"
#include "pathmanagement.h"

#include "../utilities.h"

//  the magic pattern which is used to find the correct position in the compiled executable to patch in the offset of the cabinet data is
//  defined in four parts. There may exist only one position in the whole source where these parts are concatenated - namely in the initialization
//  of the patch structrue. The check for the correct value during runtime as like as the search for the pattern inside the executable (when
//  bulding embedded installers) may only be done in parts. Otherwise, the pattern wpuld occur more than once in the binary and patching would
// not be possible!
#define	PATTERN1	"0123"
#define	PATTERN2	"4567"
#define	PATTERN3	"8d987"
#define	PATTERN4	"6543"

#define	MAGIC_PATTERN	PATTERN1 PATTERN2 PATTERN3	PATTERN4
#define	MAGIC_SIZE	(sizeof(PATTERN1)-1+sizeof(PATTERN2)-1+sizeof(PATTERN3)-1+sizeof(PATTERN4)-1)

struct installPatchStruct
{
    char searchPattern[MAGIC_SIZE+1];
    unsigned fileOffset;
};

static struct installPatchStruct patchInformation = { MAGIC_PATTERN, 0xffffffff };

static unsigned patchOffset = (unsigned long)&patchInformation.fileOffset-(unsigned long)&patchInformation.searchPattern[0];

class csvFile : public QFile
{
public:
    csvFile(QString const &filename) : QFile(filename)
    {
    }

    QString readCsvLine()
    {
        char buffer[4097];
        bool eol;
        QString result;
        do
        {
            qint64 n = readCsvLineData(buffer,4096,eol);
            buffer[n] = 0x0;
            result.append(buffer);
        } while( !eol );

        return result;
    }

private:
    qint64 readCsvLineData( char * data, qint64 maxSize, bool &EOL )
    {
        qint64 read = 0;
        int count = 0;
        EOL = false;
        do
        {
            char byte;
            if( QFile::read(&byte,1)!=1 )
			{
				EOL = true;			// end of file reached
                break;
			}
            *(data+read)=byte;

            if( byte!='\r' ) read++;                // always ignore CR
            if( byte=='\"' ) count++;               // if count of " is odd, we are inside cell definition
            if( byte=='\n' )
			{
				if( (count%2)==0 )    // ignore newline characters inside cells
					EOL = true; // ready with csv line
				else
					read--;     // just inside cell
			}
        } while( (read<maxSize) && !EOL );
        return read;
    }
};

QString getText(QString const &csvPath,QString const &cell)
{
    QString text;

    if( cell.startsWith("f:") )
    {
        QString textfile = QDir::isAbsolutePath(cell.mid(2)) ? cell.mid(2) : csvPath+"/"+cell.mid(2);
        dbgout(QString("    try opening ")+textfile);
        QFile contents(textfile);
        if( contents.open(QIODevice::ReadOnly) )
        {
            text = contents.readAll();
            contents.close();
        }
        else
            error("### can't open text file");
    }
    else
        text = cell;

    return text;
}

void copyFileEmbedPackage(QString const &srcPath,QString const &srcFile,QString const &dstPath,QString const &package)
{
    QString srcfilename = srcFile.isEmpty() ? srcPath : srcPath+"/"+srcFile;
    QString dstfilename = srcFile.isEmpty() ? dstPath : dstPath+"/"+srcFile;
    QFile src(srcfilename);
    QFile dst(dstfilename);
    if( src.open(QIODevice::ReadOnly) )
    {
        if( dst.open(QIODevice::WriteOnly|QIODevice::Truncate) )
        {
            // Don't change this code! The construction of the search pattern is intentionally split into parts! (See definition ofPATTERN1)
            QByteArray magic(PATTERN1);
			magic.append(PATTERN2);
			magic.append(PATTERN3);
			magic.append(PATTERN4);

            QByteArray arr = src.readAll();
            dst.write(arr);

            int position = arr.indexOf(magic);
            if( position>0 && (arr.indexOf(magic,position+patchOffset)<=0))
            {
				dbgout(QString("binary patch structure found at poition ")+QString::number(position)+" of file with size "+QString::number(arr.size()));

                QFile datacab(package);
                datacab.open(QIODevice::ReadOnly);
                dst.write(datacab.readAll());
                datacab.close();

				if( dst.seek(position + patchOffset) )
                {
                    int size = arr.size();
                    const char *bytes = (const char*)&size;
                    if( dst.write(bytes,4)!=4 )
                        dbgout("### couldnt patch binary!");
                }
                else
                    dbgout("### couldnt patch binary!");

                dst.setPermissions( dst.permissions() | QFile::ExeOwner | QFile::ExeGroup | QFile::ExeUser | QFile::ExeOther );
            }

            dst.close();
        }

        src.close();
    }
}
void copyApplication(QString const &srcPath,QString const &dstPath,QString const &package)
{
    QFileInfo info(srcPath);
    if( info.isFile() )
        copyFileEmbedPackage(srcPath,"",dstPath,package);
    else
    {
        QDir dir(srcPath);
        QFileInfoList list = dir.entryInfoList(QDir::Dirs | QDir::Files);

        for (int i = 0; i < list.size(); ++i)
        {
            QFileInfo fileInfo = list.at(i);

            if( fileInfo.fileName()!="." && fileInfo.fileName()!=".." )
            {
                if( fileInfo.isDir() )
                {
                    QDir d(dstPath);
                    if( !d.exists(fileInfo.fileName()) ) d.mkdir(fileInfo.fileName());

                    copyApplication(srcPath+"/"+fileInfo.fileName(),dstPath+"/"+fileInfo.fileName(),package);
                }
                else
                    copyFileEmbedPackage(srcPath,fileInfo.fileName(),dstPath,package);
            }
        }
    }
}

void addDirectory(DataCabinet &cab,QString const &baseDir,QString const &destinationBase,QString const &properties,QString const &actual,int attributes)
{
    QString myDir = actual.isNull() ? baseDir : actual;

    dbgout(QString("    searching ")+myDir+"...");

    QDir dir(myDir);
    QFileInfoList list = dir.entryInfoList(QDir::Dirs | QDir::Files);

    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);

        if( fileInfo.fileName()!="." && fileInfo.fileName()!=".." )
        {
            if( fileInfo.isDir() )
                addDirectory(cab,baseDir,destinationBase,properties,myDir + "/" + fileInfo.fileName(),attributes);
            else
            {
                QString src = myDir + "/" + fileInfo.fileName();
                QString dst = src;

                dst.remove(baseDir);
                dst = destinationBase + dst;

                //dbgout(QString("adding '")+src+"'/'"+dst+"'");
                cab.appendFileDatagram(src,dst,properties,attributes);
            }
        }
    }
}

void parseRccFile(DataCabinet &cab,QString const &baseDir,QString const &rccfile,QString const &destinationBase)
{
    QFile file(baseDir+"/"+rccfile);

    dbgout(QString("    scanning ")+rccfile+"...");

    if( file.open(QIODevice::ReadOnly) )
    {
        //cab.appendFileDatagram(baseDir+"/"+projectfile,destinationBase+"/"+projectfile,"",0);

        while( file.bytesAvailable() )
        {
            QString line = file.readLine();

            if( line.contains("<file") )
            {
                int pos1 = line.indexOf(">");
                int pos2 = line.indexOf("<",pos1+1);
                QString filename = line.mid(pos1+1,pos2-pos1-1);
                cab.appendFileDatagram(baseDir+"/"+filename,destinationBase+"/"+filename,"",0);
            }
        }
        file.close();
    }

    dbgout("    ...done.");
}
void parseQmakeProject(DataCabinet &cab,QString const &baseDir,QString const &projectfile,QString const &destinationBase)
{
    QString projectpath = QDir::isAbsolutePath(projectfile) ? projectfile : baseDir+"/"+projectfile;
    QFileInfo info(projectpath);
    QString projBase = info.dir().path();
    QString projFil = info.fileName();

    QFile file(projBase+"/"+projFil);
    bool followup = false;

    dbgout(QString("    scanning ")+projFil+"...");

    if( file.open(QIODevice::ReadOnly) )
    {
        cab.appendFileDatagram(projBase+"/"+projFil,destinationBase+"/"+projFil,"",0);

        while( file.bytesAvailable() )
        {
            QString line = file.readLine();
            int sep = line.indexOf('=');
            QString keyword;

            if( sep>0 )
                keyword = line.left(sep);
            else
                keyword = "";

            if( followup || keyword.contains("HEADERS") || keyword.contains("SOURCES") || keyword.contains("FORMS") || keyword.contains("RESOURCES")
                || keyword.contains("PRECOMPILED_HEADER") || keyword.contains("RC_FILE") || keyword.contains("ICON") || keyword.contains("QMAKE_INFO_PLIST") )
            {
                if( line.contains("\\") )
                {
                    followup = true;
                    line.remove("\\");
                }
                else
                    followup = false;

                QString names = line.mid(sep+1).simplified();

                QStringList files = names.split(" ");

                for( int i=0; i<files.count(); i++ )
                {
                    if( !files.at(i).isEmpty() )
                    {
                        QString filename = files.at(i);
                        cab.appendFileDatagram(projBase+"/"+filename,destinationBase+"/"+filename,"",0);
                        if( filename.endsWith(".qrc") )
                            parseRccFile(cab,projBase,filename,destinationBase);
                        if( filename.endsWith(".rc") )
                        {
                            QString basename = filename.left(filename.length()-3);
                            cab.appendFileDatagram(projBase+"/"+basename+".ico",destinationBase+"/"+basename+".ico","",0);
                        }
                    }
                 }
            }
        }
        file.close();
    }

    dbgout("    ...done.");
}

void cretaePackage(QString const &definitionName,QString const &packageName)
{
    csvFile file(definitionName);
    if( file.open(QIODevice::ReadOnly) )
    {
            DataCabinet cab;
        dbgVisible(true);

        QFileInfo info(definitionName);

        // skip column description
        file.readCsvLine();

        // read header definition
        QString input = file.readCsvLine();
        QStringList headers = input.remove("\"").split(";");
        dbgout(QString("Window  Title: ")+headers.at(0));
        dbgout(QString("Welcome  Text: ")+headers.at(1));
        dbgout(QString("  Setup Comp.: ")+headers.at(2));
        dbgout(QString("License  Text: ")+headers.at(3));
        dbgout(QString("Complete Text: ")+headers.at(4));

        cab.setProperty(ePropWindowTitle,getText(info.dir().path(),headers.at(0))); // Fenstername
        cab.setProperty(ePropWelcomeText,getText(info.dir().path(),headers.at(1))); // Text für Willkommensseite
        cab.setProperty(ePropCompletionText,getText(info.dir().path(),headers.at(2))); // Text für Completion
        cab.setProperty(ePropLicenceText,getText(info.dir().path(),headers.at(3))); // Text für Lizenz
        cab.setProperty(ePropComponentDefinition,headers.at(4)); // Definition Komponenten

        QString target;
#ifdef Q_OS_WIN32
        target = "win32";
#endif
#ifdef Q_OS_DARWIN
        target = "mac";
#endif

        cab.createFile(packageName);

        // skip column description
        file.readCsvLine();

        while( file.bytesAvailable() )
        {
            // read entry
            QString input = file.readCsvLine();
            QStringList cells = input.split(";");

            // skip comment lines; take entry only if suitable target
            if( input.at(0)!='#' )
            {
                if( cells.at(5).simplified().isEmpty() || cells.at(5).contains(target) )
                {
                    QString srcpath = QDir::isAbsolutePath(cells.at(1)) ? cells.at(1) : info.dir().path()+"/"+cells.at(1);
                    if( cells.at(0)=="d" )
                    {
                        int attributes = DatagramFileHandler::getPropertiesFromString(cells.at(3));
                        addDirectory(cab,srcpath,cells.at(2),cells.at(4),QString::null,attributes);
                    }
                    if( cells.at(0).left(1)=="f" )
                    {
                        int attributes = DatagramFileHandler::getPropertiesFromString(cells.at(3));
                        if( cells.at(0).mid(1,1)=="r" )
                            cab.appendFileDatagram("",cells.at(2),cells.at(4),removeDestination);
                        else
                            cab.appendFileDatagram(srcpath,cells.at(2),cells.at(4),attributes);
                    }
                    if( cells.at(0).left(1)=="s" )
                    {
                        if( cells.at(0).mid(1,2)=="oa" )
                            cab.appendSettingsDatagram(cells.at(1),cells.at(2),cells.at(4),settingsAppAndOrgName);
                        else
                            cab.appendSettingsDatagram(cells.at(1),cells.at(2),cells.at(4),cells.at(3).toInt());
                    }
                    if( cells.at(0).left(1)=="l" )
                    {
                        DatagramLinksHandler::linkCommand cmd = DatagramLinksHandler::eCreateStartupLink;
                        if( cells.at(0).mid(1,2)=="cs" )
                            cmd = DatagramLinksHandler::eCreateStartupLink;
                        if( cells.at(0).mid(1,2)=="cd" )
                            cmd = DatagramLinksHandler::eCreateDesktopLink;
                        if( cells.at(0).mid(1,2)=="rs" )
                            cmd = DatagramLinksHandler::eRemoveStartupLink;
                        if( cells.at(0).mid(1,2)=="rd" )
                            cmd = DatagramLinksHandler::eRemoveDesktopLink;
                        cab.appendLinksDatagram(cmd,cells.at(2),cells.at(1),cells.at(4),cells.at(3).toInt());
                    }
                    if( cells.at(0).left(1)=="p" )
                    {
                        QFileInfo info2(srcpath);
                        parseQmakeProject(cab,info.dir().path(),cells.at(1),cells.at(2));
                    }
                }
                else
                    dbgout(QString("... skipping '")+input.simplified()+"'...");
            }
        }
        cab.closeFile();

        file.close();
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("VISolutions.de");
    QCoreApplication::setApplicationName("QtInstall");

    PathManagement::init();

	bool installingPackage = true;
    QString selectedPackage;
    QString definitionName;

    // we have to access both members of the patch structure here. Otherwise, optimizing compilers would drop the pattern data out of the executable.
    // Don't change this code! The comparison of the path pattern is intentionally split into parts! (See definition ofPATTERN1)
    int idx = 0; bool patternOk = true; int len = sizeof(PATTERN1)-1;
    patternOk = patternOk && (strncmp(patchInformation.searchPattern,PATTERN1,len)==0); idx+=len; len = sizeof(PATTERN2)-1;
    patternOk = patternOk && (strncmp(patchInformation.searchPattern+idx,PATTERN2,len)==0); idx+=len; len = sizeof(PATTERN3)-1;
    patternOk = patternOk && (strncmp(patchInformation.searchPattern+idx,PATTERN3,len)==0); idx+=len; len = sizeof(PATTERN4)-1;
    patternOk = patternOk && (strncmp(patchInformation.searchPattern+idx,PATTERN4,len)==0);
    if( patternOk && (patchInformation.fileOffset!=0xffffffff) )
    {
        selectedPackage = argv[0];
    }
    else
    {
        if( QFile::exists("definition.csv") )
        {
            if( QMessageBox::question(0,"found definition.csv file","Do you want to create a\nQtInstall package form the\ngiven 'definition.csv' file ?",
                                  QMessageBox::Yes | QMessageBox::No)==QMessageBox::Yes )
                installingPackage = false;

            if( !installingPackage )
                definitionName = "definition.csv";
        }
    }

        if( installingPackage )
        {
            if( selectedPackage.isEmpty() )
            {
                QDir actual("");
                QStringList filter; filter << "*.qip";
                QStringList found = actual.entryList(filter);

                if( found.isEmpty() || found.count()>1 )
                    selectedPackage = QFileDialog::getOpenFileName(0,"Select Installation Package","","QtInstall Files (*.qip)"/*,0,QFileDialog::DontUseNativeDialog*/);
                else
                    selectedPackage = found.at(0);
            }

            if( !selectedPackage.isEmpty() )
            {
                DataCabinet cab;
                Wizard w(&cab);

				int fileOffset = patchInformation.fileOffset;

                if( cab.openFile(selectedPackage,fileOffset) )
                {
                    w.setWindowTitle(cab.getProperty(ePropWindowTitle));
                    w.show();
                    w.exec();
                }
            }
            else
                installingPackage = false;
        }
        if( !installingPackage )
        {
#ifdef Q_OS_WIN32
            QString packageFilter = "QtInstall packages (*.qip);;QtInstall standalone application (*.exe)";
#endif
#ifdef Q_OS_DARWIN
            QString packageFilter = "QtInstall packages (*.qip);;QtInstall standalone application (*.app)";
#endif
            QString packageName;

            if( definitionName.isEmpty() )
                definitionName = QFileDialog::getOpenFileName(0,"Select Installation Definition","","QtInstall Definitions (*.csv)"/*,0,QFileDialog::DontUseNativeDialog*/);

            QString selectedFilter;
            if( !installingPackage && !definitionName.isEmpty() )
                packageName = QFileDialog::getSaveFileName(0,"Save Installation Package As","",packageFilter,&selectedFilter/*,QFileDialog::DontUseNativeDialog*/,QFileDialog::DontConfirmOverwrite);

            if(!packageName.isEmpty() )
            {
                QString selectedExtension;
                bool createEmbedded = false;

                if( selectedFilter.contains(".qip") )
                    selectedExtension = ".qip";
                else if( selectedFilter.contains(".app") )
                {
                    selectedExtension = ".app";
                    createEmbedded = true;
                }
                else if( selectedFilter.contains(".exe") )
                {
                    selectedExtension = ".exe";
                    createEmbedded = true;
                }

                if( !packageName.endsWith(selectedExtension) )
                    packageName.append(selectedExtension);

                if( !QFile::exists(packageName) ||
                    QMessageBox::information(0,"warning","the file you have chosen already exists.\nWould you ike to overwrite it?",
                                             QMessageBox::Yes,QMessageBox::No) == QMessageBox::Yes )
                {
                    if( createEmbedded )
                    {
                        cretaePackage(definitionName,"temp.qip");

#ifdef Q_OS_WIN32
                        QString myOwnApp = argv[0];
#endif
#ifdef Q_OS_DARWIN
                        QDir d(packageName);
                        d.mkpath(packageName);

                        QString myOwnApp = QString(argv[0]).remove("/Contents/MacOS/QtInstall");
#endif
                        copyApplication(myOwnApp,packageName,"temp.qip");

                        QFile::remove("temp.qip");
                    }
                    else
                        cretaePackage(definitionName,packageName);

                    QMessageBox::information(0,"processing finished","definition file succesfully processed.");
                }
            }
        }

    if( dbgVisible() )
        return a.exec();
}

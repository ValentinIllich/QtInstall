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

class csvFile : public QFile
{
public:
   csvFile(QString const &filename) : QFile(filename)
   {
   }
//protected:
    qint64 readLineDataEx ( char * data, qint64 maxSize )
    {
        qint64 read = 0;
        qint64 block = 0;
        int count = 0;
/*        do
        {
            // if we don't call the base implementation, the last line will not be recognized.
            block = QFile::readLineData(data+read,maxSize-read);
            if( block>0 )
            {
                read += block;
                count = 0;
                for( int i=0; i<read; i++ )
                    if( data[i]=='\"' ) count++;
            }
        } while( (block>0) && ((data[read-1]!='\n') || (count%2)!=0) );*/
		bool noEOL = true;
		do
		{
			char byte;
			if( QFile::read(&byte,1)!=1 )
				break;

			*(data+read)=byte;
			
			if( byte!='\r' ) read++;
			if( byte=='\"' ) count++;
			if( (byte=='\n') && ((count%2)==0) ) noEOL = false;
		} while( (read<maxSize) && noEOL );
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
            dbgout("### can't open text file");
    }
    else
        text = cell;

    return text;
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


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DataCabinet cab;
    Wizard w(&cab);

    PathManagement::init();

    bool createInstaller = false;
    QString definitionName;
    QString packageName;
    QString selectedFile;

    if( QFile::exists("definition.csv") )
    {
        if( QMessageBox::question(0,"found definition.csv file","Do you want to create a\nQtInstall package form the\ngiven 'definition.csv' file ?",
                              QMessageBox::Yes | QMessageBox::No)==QMessageBox::Yes )
            createInstaller = true;

        if( createInstaller )
            definitionName = "definition.csv";
    }

    if( !createInstaller )
    {
        QDir actual("");
        QStringList filter; filter << "*.qip";
        QStringList found = actual.entryList(filter);

        if( found.isEmpty() || found.count()>1 )
            selectedFile = QFileDialog::getOpenFileName(0,"Select Installation Package","","QtInstall Files (*.qip)"/*,0,QFileDialog::DontUseNativeDialog*/);
        else
            selectedFile = found.at(0);

        if( selectedFile.isEmpty() )
        {
            createInstaller = true;
            definitionName = QFileDialog::getOpenFileName(0,"Select Installation Definition","","QtInstall Definitions (*.csv)"/*,0,QFileDialog::DontUseNativeDialog*/);
        }
    }

    if( createInstaller && !definitionName.isEmpty() )
        packageName = QFileDialog::getSaveFileName(0,"Save Installation Package As","","QtInstall packages (*.qip)"/*,0,QFileDialog::DontUseNativeDialog*/);

    if( createInstaller && !packageName.isEmpty() )
    {
        csvFile file(definitionName);
        if( file.open(QIODevice::ReadOnly) )
        {
            dbgVisible(true);

            QFileInfo info(definitionName);

            // skip column description
            file.readLine(4096);

            // read header definition
            //QString input = file.readLine(4096);
			char buffer[4097];
			qint64 n = file.readLineDataEx(buffer,4096);
			buffer[n] = 0x0;
			QString input(buffer);
dbgout(input);
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

            cab.createFile(packageName);

            QString target;
#ifdef Q_OS_WIN32
            target = "win32";
#endif
#ifdef Q_OS_DARWIN
            target = "mac";
#endif

            // skip column description
            file.readLine(4096);

            while( file.bytesAvailable() )
            {
                // read entry
                QString input = file.readLine(4096);
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

            QMessageBox::information(0,"processing finished","definition file succesfully processed.");
        }
    }
    else if( !selectedFile.isEmpty() )
    {
        if( cab.openFile(selectedFile) )
        {
            w.setWindowTitle(cab.getProperty(ePropWindowTitle));
            w.show();
            w.exec();
        }
    }

    if( dbgVisible() )
        return a.exec();
}

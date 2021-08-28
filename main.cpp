#include <QApplication>
#include <QDir>
#include <QFileInfoList>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>

#include "wizard.h"
#include "datacabinet.h"
#include "datagramfilehandler.h"
#include "datagramlinkshandler.h"
#include "pathmanagement.h"

#include "../backup/Utilities.h"

extern "C" void checkForPasswdHelper(int argc, char **argv);
extern "C" int hasAdminRights(int argc, char* argv[]);
extern "C" int getAdminRights(int argc, char* argv[], char *password);

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

static unsigned patchOffset = (unsigned)(quint64)&patchInformation.fileOffset-(quint64)&patchInformation.searchPattern[0];
static char *m_argv0;

static char lastbyte = 0x0;

csvFile::csvFile(QString const &filename) : QFile(filename)
{
	lastbyte = 0x0;
}

QString csvFile::readCsvLine()
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

qint64 csvFile::readCsvLineData( char * data, qint64 maxSize, bool &EOL )
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

		if( byte=='\"' ) count++;                               // if count of " is odd, we are inside cell definition

		bool endOfLine = false;
		if( byte=='\r' )
			endOfLine = true;
		else if( byte=='\n' && lastbyte!='\r' )
			endOfLine = true;

		lastbyte = byte;

		if( endOfLine )
		{
			if( (count%2)==0 )    // ignore newline characters enclosed in '"'
				EOL = true; // ready with csv line
		}
		else if( byte!='\n' )
			read++;
	} while( (read<maxSize) && !EOL );
  return read;
}

QString getText(QString const &csvPath,QString const &cell)
{
    QString text;

    if( cell.startsWith("f:") )
    {
        QString textfile = QDir::isAbsolutePath(cell.mid(2)) ? cell.mid(2) : csvPath+"/"+cell.mid(2);
        dbgout(QString("    try opening ")+textfile,2);
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
                dbgout(QString("binary patch structure found at poition ")+QString::number(position)+" of file with size "+QString::number(arr.size()),2);

                QFile datacab(package);
                datacab.open(QIODevice::ReadOnly);
                dst.write(datacab.readAll());
                datacab.close();

				if( dst.seek(position + patchOffset) )
                {
                    int size = arr.size();
                    const char *bytes = (const char*)&size;
                    if( dst.write(bytes,4)!=4 )
                        dbgout("### couldnt patch binary!",0);
                }
                else
                    dbgout("### couldnt patch binary!",0);

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

    dbgout(QString("    searching ")+myDir+"...",2);

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

                //dbgout(QString("adding '")+src+"'/'"+dst+"'",0);
                cab.appendFileDatagram(src,dst,properties,attributes);
            }
        }
    }
}

void parseRccFile(DataCabinet &cab,QString const &baseDir,QString const &rccfile,QString const &destinationBase)
{
    QFile file(baseDir+"/"+rccfile);

    dbgout(QString("    scanning ")+rccfile+"...",2);

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

    dbgout("    ...done.",2);
}
void parseQmakeProject(DataCabinet &cab,QString const &baseDir,QString const &projectfile,QString const &destinationBase)
{
    QString projectpath = QDir::isAbsolutePath(projectfile) ? projectfile : baseDir+"/"+projectfile;
    QFileInfo info(projectpath);
    QString projBase = info.dir().path();
    QString projFil = info.fileName();

    QFile file(projBase+"/"+projFil);
    bool followup = false;

    dbgout(QString("    scanning ")+projFil+"...",2);

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

    dbgout("    ...done.",2);
}

bool cretaePackage(QString const &definitionName,QString const &packageName)
{
  bool ret = false;

  csvFile file(definitionName);
  if( file.open(QIODevice::ReadOnly) )
  {

      DataCabinet cab;
      dbgVisible(true);

      QFileInfo info(definitionName);

      // check header definition
      QString header = file.readCsvLine();
      if( header.contains("WindowTitle") )
      {
          // old version: UUID, version etc not given
          cab.setProperty(ePropSetupId,definitionName/*"88C45A0C-39E4-4EC8-9A47-8A75AE5120CD"*/);
          cab.setProperty(ePropSetupMajor,"0");
          cab.setProperty(ePropSetupMinor,"0");
      }
      else
      {
          // new version: extended csv format
          QString input = file.readCsvLine();
          QStringList specs = input.remove("\"").split(";");

          cab.setProperty(ePropSetupId,specs.at(1));
          cab.setProperty(ePropSetupMajor,specs.at(0));
          cab.setProperty(ePropSetupMinor,specs.at(3));

          // skip column description
          file.readCsvLine();
      }

      // read header definition
      QString input = file.readCsvLine();
      QStringList headers = input.remove("\"").split(";");
      dbgout(QString("  Setup  UUID: ")+cab.getProperty(ePropSetupId),1);
      dbgout(QString("  Setup Major: ")+cab.getProperty(ePropSetupMajor),1);
      dbgout(QString("  Setup Minor: ")+cab.getProperty(ePropSetupMinor),1);
      dbgout(QString("Window  Title: ")+headers.at(0),1);
      dbgout(QString("Welcome  Text: ")+headers.at(1),1);
      dbgout(QString("  Setup Comp.: ")+headers.at(2),1);
      dbgout(QString("License  Text: ")+headers.at(3),1);
      dbgout(QString("Complete Text: ")+headers.at(4),1);

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

      if( !cab.createFile(packageName) )
      {
          file.close();
          return false;
      }

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
              if( cells.size()>=6 && (cells.at(5).simplified().isEmpty() || cells.at(5).contains(target)) )
              {
                  QString srcpath = QDir::isAbsolutePath(cells.at(1)) ? cells.at(1) : info.dir().path()+"/"+cells.at(1);
                  if( cells.at(0)=="d" )
                  {
                      int attributes = DatagramFileHandler::getPropertiesFromString(cells.at(3));
                      addDirectory(cab,srcpath,cells.at(2),cells.at(4),QString(),attributes);
                  }
                  else if( cells.at(0).left(1)=="f" )
                  {
                      int attributes = DatagramFileHandler::getPropertiesFromString(cells.at(3));
                      if( cells.at(0).mid(1,1)=="r" )
                          cab.appendFileDatagram("",cells.at(2),cells.at(4),removeDestination);
                      else
                          cab.appendFileDatagram(srcpath,cells.at(2),cells.at(4),attributes);
                  }
                  else if( cells.at(0).left(1)=="s" )
                  {
                      if( cells.at(0).mid(1,2)=="oa" )
                          cab.appendSettingsDatagram(cells.at(1),cells.at(2),cells.at(4),settingsAppAndOrgName);
                      else
                          cab.appendSettingsDatagram(cells.at(1),cells.at(2),cells.at(4),cells.at(3).toInt());
                  }
                  else if( cells.at(0).left(1)=="l" )
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
                  else if( cells.at(0).left(1)=="p" )
                  {
                      QFileInfo info2(srcpath);
                      parseQmakeProject(cab,info.dir().path(),cells.at(1),cells.at(2));
                  }
                  else
                      dbgout(QString("#### unrecognized package code: '")+cells.at(0)+"'...",0);
              }
              else
                  dbgout(QString("... skipping '")+input.simplified()+"'...",2);
          }
      }
      cab.closeFile();

      file.close();

    ret = true;
  }
  else
    dbgout(QString("#### file not found: '")+definitionName.simplified()+"'.",0);

  return ret;
}

void createSetup(QString const &definitionName)
{
#ifdef Q_OS_WIN32
			QString packageFilter = "QtInstall packages (*.qip);;QtInstall standalone application (*.exe)";
#endif
#ifdef Q_OS_DARWIN
			QString packageFilter = "QtInstall packages (*.qip);;QtInstall standalone application (*.app)";
#endif
	QString selectedFilter;

	QString packageName = QFileDialog::getSaveFileName(0,"Save Installation Package As","",packageFilter,&selectedFilter/*,QFileDialog::DontUseNativeDialog*/,QFileDialog::DontConfirmOverwrite);

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
        QString temp = QDir::tempPath();
        if( !temp.endsWith("/") ) temp += "/";
        temp += "temp.qip";

        cretaePackage(definitionName,temp);

#ifdef Q_OS_WIN32
				QString myOwnApp = m_argv0;
#endif
#ifdef Q_OS_DARWIN
				QDir d(packageName);
				d.mkpath(packageName);

				QString myOwnApp = QString(m_argv0).remove("/Contents/MacOS/QtInstall");
#endif
        copyApplication(myOwnApp,packageName,temp);

        QFile::remove(temp);
			}
			else
				cretaePackage(definitionName,packageName);

			QMessageBox::information(0,"processing finished","definition file succesfully processed.");
		}
	}
}

int main(int argc, char *argv[])
{
  checkForPasswdHelper(argc,argv);

  bool ok = true;
  QString text = "";

  if( argc==2 )
    qDebug("++++ running as admin");

  QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("VISolutions.de");
    QCoreApplication::setApplicationName("QtInstall");

	m_argv0 = argv[0];

    PathManagement::init();

  bool installingPackage = true;
    QString selectedPackage;

    // we have to access both members of the patch structure here. Otherwise, optimizing compilers would drop the pattern data out of the executable.
    // Don't change this code! The comparison of the path pattern is intentionally split into parts! (See definition of PATTERN1)
    int idx = 0; bool patternOk = true; int len = sizeof(PATTERN1)-1;
    patternOk = patternOk && (strncmp(patchInformation.searchPattern,PATTERN1,len)==0); idx+=len; len = sizeof(PATTERN2)-1;
    patternOk = patternOk && (strncmp(patchInformation.searchPattern+idx,PATTERN2,len)==0); idx+=len; len = sizeof(PATTERN3)-1;
    patternOk = patternOk && (strncmp(patchInformation.searchPattern+idx,PATTERN3,len)==0); idx+=len; len = sizeof(PATTERN4)-1;
    patternOk = patternOk && (strncmp(patchInformation.searchPattern+idx,PATTERN4,len)==0);
    if( patternOk && (patchInformation.fileOffset!=0xffffffff) )
    {
        // embedded package found; the installer executable itself is the package
        selectedPackage = argv[0];
    }
    else
    {
        if( QFile::exists("definition.csv") )
        {
            if( QMessageBox::question(0,"found definition.csv file","Do you want to create a\nQtInstall package form the\ngiven 'definition.csv' file ?",
                                  QMessageBox::Yes | QMessageBox::No)==QMessageBox::Yes )
            installingPackage = false;
        }
    }

    bool tryAdmin = false;

    if( installingPackage )
    {
      // we want to install a package from given file
        if( selectedPackage.isEmpty() )
        {
            // package file not yet specified: show file selector
            QDir actual("");
            QStringList filter; filter << "*.qip";
            QStringList found = actual.entryList(filter);

            if( found.count()>1 )
                selectedPackage = QFileDialog::getOpenFileName(0,"Select Installation Package","","QtInstall Files (*.qip)"/*,0,QFileDialog::DontUseNativeDialog*/);
            else if( found.count()==1 )
                selectedPackage = found.at(0);
        }

        if( !selectedPackage.isEmpty() )
        {
            if( !hasAdminRights(argc,argv) )
            {
#if defined(Q_OS_MAC)  || defined(Q_OS_LINUX)
                text = QInputDialog::getText(0,"Authorization",
                                               "Enter Administrator Password:", QLineEdit::Password,
                                               "", &ok);
                if (text.isEmpty())
                    ok = false;
#endif
                if( ok )
                    getAdminRights(argc,argv,text.toLatin1().data());
            }

            // do the installation
            DataCabinet cab;

            int fileOffset = patchInformation.fileOffset;

            //getAdminRights(argc,argv);

            if( cab.openFile(selectedPackage,fileOffset) )
            {
                Wizard w(&cab);
                w.setWindowTitle(cab.getProperty(ePropWindowTitle)+" V"+cab.getProperty(ePropSetupMajor)+"."+cab.getProperty(ePropSetupMinor));
                w.show();
                w.exec();

                if( cab.hasError() )
                    tryAdmin = true;
            }
        }
        else
        {
            // no file given; switch to package creation mode and show wizard
            Wizard wizz(0,0);
            wizz.setWindowTitle(QString("QtInstall ")+QtInstallVersion+" creating packages");
            wizz.show();
            return qApp->exec();
        }
    }
    else
    {
        //createPackage("definition.csv");
    }

    if( dbgVisible() )
        return a.exec();

    if( tryAdmin )
    {
        if( QMessageBox::question(0,"admin access needed","Do you want to restart the installer as administrator ?",
                                  QMessageBox::Yes | QMessageBox::No)==QMessageBox::Yes )
            ;//getAdminRights(argc,argv);
    }

    return 0;
}

#include "wizard.h"

#include "licensepage.h"
#include "welcomepage.h"
#include "installpage.h"
#include "completepage.h"
#include "pathmanagement.h"

#include <QtGui>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

#include <QProcess>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>

#include "../backup/Utilities.h"

extern void showHelp(QObject *parent,QString const &ressourcePath,QString const &helpfile);

ConfigPage::ConfigPage()
{
  QLabel *m_label = new QLabel("<h3>Package Wizard</h3>This wizard helps you with the creation of QtInstall packages. Please select a .csv definition file, preview the result or create a package.");
  m_label->setWordWrap(true);
  m_label->setTextFormat(Qt::RichText);

  m_filename = new QLabel("<none>");
  //    m_filename->setAlignment(Qt::AlignRight);
  m_preview = new QPushButton("Preview package...");
  m_deatailed = new QCheckBox("detailed logging");
  m_fileselect = new QPushButton("Definition file...");
  m_create = new QPushButton("Create package...");

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(m_label);

  QHBoxLayout *row1 = new QHBoxLayout;
  //         row1->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::MinimumExpanding));
  //     row1->addWidget(new QLabel("definition file:"));
  row1->addWidget(m_fileselect);
  row1->addWidget(m_filename,1);

  QHBoxLayout *row2 = new QHBoxLayout;
  row2->addWidget(m_preview);
  row2->addWidget(m_deatailed);
  row2->addWidget(new QLabel(""),1);
  row2->addWidget(m_create);

  m_edit = new QPlainTextEdit();
  m_edit->setReadOnly(true);
  m_edit->setLineWrapMode(QPlainTextEdit::NoWrap);
  //m_edit->setFrameStyle(QFrame::NoFrame);

  layout->addLayout(row1);
  layout->addLayout(row2);
  layout->addWidget(m_edit);

  setLayout(layout);
}
ConfigPage::~ConfigPage()
{
  setDbgWindow(NULL);
}


void ConfigPage::initializePage()
{
  connect(m_preview,SIGNAL(clicked()),this,SLOT(showPreview()));
  connect(m_fileselect,SIGNAL(clicked()),this,SLOT(selectFile()));
  connect(m_create,SIGNAL(clicked()),this,SLOT(create()));
}

bool ConfigPage::validatePage()
{
  return true;
}

bool cretaePackage(QString const &definitionName,QString const &packageName);
void createSetup(QString const &definitionName);

void ConfigPage::selectFile()
{
  QString filename = QFileDialog::getOpenFileName(this,"package definition","","QtInstall definition files (*.csv)");
  if( !filename.isEmpty() )
    m_filename->setText(filename);
}
void ConfigPage::create()
{
  m_edit->clear();
  setDbgWindow(m_edit,1);
  createSetup(m_filename->text());
}

void ConfigPage::showPreview()
{
  m_edit->clear();
  setDbgWindow(m_edit,m_deatailed->isChecked() ? 3: 1);

  QString temp = QDir::tempPath();
  if( !temp.endsWith("/") ) temp += "/";
  temp += "temp.qip";

  if( cretaePackage(m_filename->text(),temp) )
  {
    DataCabinet *cab = new DataCabinet;

    int fileOffset = 0;

    if( cab->openFile(temp,fileOffset) )
    {
      Wizard *w = new Wizard(cab);
      w->setWindowTitle(cab->getProperty(ePropWindowTitle)+" V"+cab->getProperty(ePropSetupMajor)+"."+cab->getProperty(ePropSetupMinor));
      w->setDebugMode(true);
      connect(w,SIGNAL(accepted()),w,SLOT(deleteLater()));
      connect(w,SIGNAL(rejected()),w,SLOT(deleteLater()));
      w->show();
    }

    QFile::remove(temp);
  }
}

/**********************************************************************************/

Wizard::Wizard(DataCabinet *cab,QWidget *parent)
  : QWizard(parent)
  , m_cabinet(cab)
  , m_isMajorUpdate(false)
  , m_isMinorUpdate(false)
  , m_debugging(false)
{
  if( cab!=NULL )
  {
    setWizardStyle(QWizard::ClassicStyle);
    //setPixmap(QWizard::WatermarkPixmap,QPixmap(50,20));
    cab->isUpdateInstallation(m_isMajorUpdate,m_isMinorUpdate);

    if( m_isMajorUpdate || m_isMinorUpdate )
    {
      addPage(new WelcomePage(m_cabinet));
      if( !m_cabinet->getProperty(ePropLicenceText).isEmpty() )
        addPage(new LicensePage(m_cabinet));
    }
    addPage(new InstallPage(m_cabinet));
    addPage(new CompletePage(m_cabinet));
  }
  else
  {
    setWizardStyle(QWizard::ClassicStyle);
    setOption(QWizard::HaveHelpButton);

    installSettings sett;
    if( sett.value("QtInstallVersion","").isEmpty() )
      addPage(new LicensePage());

    addPage(new ConfigPage());
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect size = screen->geometry();
    resize(size.width()/3*2,size.height()/2);
  }

  connect(this, SIGNAL(helpRequested()), this, SLOT(showHelp()));
  connect(this, SIGNAL(customButtonClicked(int)), this, SLOT(showCustom(int)));
}

Wizard::~Wizard()
{
  if( m_debugging )
  {
    m_cabinet->closeFile();
    delete m_cabinet;
    QFile::remove("temp.qip");
  }
}

void Wizard::setDebugMode(bool debugging)
{
  m_debugging = debugging;
  if( m_cabinet )
    m_cabinet->setDebugMode(debugging);

}

void Wizard::closeEvent ( QCloseEvent * event )
{
  if( m_debugging )
    deleteLater();
  else
    QWidget::closeEvent(event);
}

/** this method will display a custom button */
void Wizard::showCustom(int which)
{
  switch( which )
  {
        case CustomButton1:
    dbgVisible(true);
    accept();
    break;
  }
}

void Wizard::showHelp()
{
    ::showHelp(this,":/documentation/QtInstall.pdf","QtInstall.pdf");
  }

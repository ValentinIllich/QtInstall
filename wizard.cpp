#include "wizard.h"

#include "licensepage.h"
#include "welcomepage.h"
#include "installpage.h"
#include "completepage.h"


#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QSettings>

#include "../utilities.h"

extern void showHelp(QObject *parent,QString const &ressourcePath,QString const &helpfile);

ConfigPage::ConfigPage()
{
    QLabel *m_label = new QLabel("<h3>Package Wizard</h3>This wizard helps you with the creation of QtInstall packages. Please select a .csv definition file, preview the result or create a package.");
	m_label->setWordWrap(true);
	m_label->setTextFormat(Qt::RichText);

    m_filename = new QLabel("<none>");
//    m_filename->setAlignment(Qt::AlignRight);
    m_preview = new QPushButton("Preview package...");
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
     row2->addWidget(new QLabel(""),1);
	 row2->addWidget(m_create);

     QPlainTextEdit *m_edit = new QPlainTextEdit();
     m_edit->setReadOnly(true);
	 m_edit->setLineWrapMode(QPlainTextEdit::NoWrap);
	 //m_edit->setFrameStyle(QFrame::NoFrame);
	 setDbgWindow(m_edit);

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
	createSetup(m_filename->text());
}

void ConfigPage::showPreview()
{
	if( cretaePackage(m_filename->text(),"temp.qip") )
	{
		DataCabinet *cab = new DataCabinet;
		Wizard *w = new Wizard(cab);
		w->setDebugMode(true);

		int fileOffset = 0;

		if( cab->openFile("temp.qip",fileOffset) )
		{
			w->setWindowTitle(cab->getProperty(ePropWindowTitle));
			connect(w,SIGNAL(accepted()),w,SLOT(deleteLater()));
			connect(w,SIGNAL(rejected()),w,SLOT(deleteLater()));
			w->show();
		}
	}
}

/**********************************************************************************/

Wizard::Wizard(DataCabinet *cab,QWidget *parent)
    : QWizard(parent)
    , m_cabinet(cab)
	, m_debugging(false)
{
    if( cab!=NULL )
    {
        addPage(new WelcomePage(m_cabinet));
        if( !m_cabinet->getProperty(ePropLicenceText).isEmpty() )
            addPage(new LicensePage(m_cabinet));
        addPage(new InstallPage(m_cabinet));
        addPage(new CompletePage(m_cabinet));
    }
    else
    {
        setOption(QWizard::HaveHelpButton);

        QSettings sett;
        if( sett.value("QtInstallVersion","").toString()!=QtInstallVersion )
            addPage(new LicensePage());

        addPage(new ConfigPage());
		QRect size = qApp->desktop()->screenGeometry(0);
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

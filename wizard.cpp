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

Wizard::Wizard(DataCabinet *cab,QWidget *parent)
    : QWizard(parent)
    , m_cabinet(cab)
{
    setOption(QWizard::HaveHelpButton);

    QSettings sett;
    if( sett.value("QtInstallVersion","").toString()!=QtInstallVersion )
        addPage(new LicensePage());

    addPage(new WelcomePage(m_cabinet));
    if( !m_cabinet->getProperty(ePropLicenceText).isEmpty() )
        addPage(new LicensePage(m_cabinet));
    addPage(new InstallPage(m_cabinet));
    addPage(new CompletePage(m_cabinet));

    connect(this, SIGNAL(helpRequested()), this, SLOT(showHelp()));
    connect(this, SIGNAL(customButtonClicked(int)), this, SLOT(showCustom(int)));
}

Wizard::~Wizard()
{
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
}

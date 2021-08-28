#include "installpage.h"

#include <qtgui>

#include "../utilities.h" // dbgout()

InstallPage::InstallPage(DataCabinet *cab)
    : m_cabinet(cab)
    , m_isRemoving(false)
    , m_isMajorUpdate(false)
    , m_isMinorUpdate(false)
{
    cab->isUpdateInstallation(m_isMajorUpdate,m_isMinorUpdate);
    m_isRemoving = ( !m_isMajorUpdate && !m_isMinorUpdate );

    m_label = new QLabel();
    m_label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_label);

    if( m_isMajorUpdate )
        m_label->setText("the installer is now ready for replacing the needed files onto your hard disk.");
    else if( m_isMinorUpdate )
        m_label->setText("the installer is now ready for copying the needed files onto your hard disk.");
    else
        m_label->setText("the installer is now ready for removing all needed file from your hard disk.");

    //layout->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding));

    m_bar = new QProgressBar;
    m_bar->setMinimum(0);
    m_bar->setMaximum(100);
    layout->addWidget(m_bar);
    setLayout(layout);
}

void InstallPage::setProgress(int progress)
{
    dbgout(QString("...progress=")+QString::number(progress),3);
    m_bar->setValue(progress);
    qApp->processEvents();
}

void InstallPage::initializePage()
{
    if( m_isMajorUpdate )
        setTitle("Ready For Upgrade");
    else if( m_isMinorUpdate )
        setTitle("Ready For Installation");
    else
        setTitle("Ready For Removing Software");
}

bool InstallPage::validatePage()
{
    m_cabinet->setProgressHandler(this);

    if( m_isMajorUpdate || m_isRemoving )
    {
        m_cabinet->undoInstallation();
        m_bar->setValue(0);
    }

    if( m_isMajorUpdate || m_isMinorUpdate )
    {
        m_cabinet->scanFile();
        m_cabinet->closeFile();
    }

    m_cabinet->setProgressHandler(NULL);

    return true;
}

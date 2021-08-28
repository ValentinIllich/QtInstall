#include "installpage.h"

#include <qtgui>

#include "../utilities.h" // dbgout()

InstallPage::InstallPage(DataCabinet *cab)
    : m_cabinet(cab)
    , m_isRemoving(false)
    , m_isMajorUpdate(false)
    , m_isMinorUpdate(false)
    , m_box(NULL)
    , m_rb1(NULL)
    , m_rb2(NULL)
{
    cab->isUpdateInstallation(m_isMajorUpdate,m_isMinorUpdate);

    m_label = new QLabel();
    m_label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_label);

    if( m_isMajorUpdate )
        m_label->setText("the installer is now ready for replacing the needed files on your hard disk.");
    else if( m_isMinorUpdate )
        m_label->setText("the installer is now ready for copying the needed files onto your hard disk.");
    else
    {
        m_label->setText("the installer is now ready for modifying the installation on your hard disk.");
//    layout->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding));

        layout->addWidget(new QLabel());

        m_rb2 = new QRadioButton("repair current installation");
        layout->addWidget(m_rb2);
        m_rb1 = new QRadioButton("remove software from disk");
        layout->addWidget(m_rb1);
        m_rb2->setChecked(true);

//        layout->addWidget(new QLabel());

        m_box = new QCheckBox("show maintenance protocol");
        layout->addWidget(m_box);
    }

    layout->addWidget(new QLabel());

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
        setTitle("Ready For Upgrading "+m_cabinet->getProperty(ePropWindowTitle));
    else if( m_isMinorUpdate )
        setTitle("Ready For Installing "+m_cabinet->getProperty(ePropWindowTitle));
    else
        setTitle("Ready For Modifying "+m_cabinet->getProperty(ePropWindowTitle));
}

bool InstallPage::validatePage()
{
    if( m_rb2 && m_rb2->isChecked() )
    {
        m_isMinorUpdate = true;
        m_isRemoving = false;
    }
    else
        m_isRemoving = !m_isMajorUpdate && !m_isMinorUpdate;

    if( m_box && m_box->isChecked() )
    {
        setDbgWindow(NULL,1);
        dbgVisible(true);
    }

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

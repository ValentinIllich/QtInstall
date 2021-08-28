#include "installpage.h"

#include <qtgui>

InstallPage::InstallPage(DataCabinet *cab)
    : m_cabinet(cab)
{
    m_label = new QLabel("the installer is now ready for copying the needed files onto your hard disk.");
    m_label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_label);

    //layout->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding));

    m_bar = new QProgressBar;
    m_bar->setMinimum(0);
    m_bar->setMaximum(100);
    layout->addWidget(m_bar);
    setLayout(layout);
}

void InstallPage::setProgress(int progress)
{
    dbgout(QString("progress=")+QString::number(progress));
    m_bar->setValue(progress);
    qApp->processEvents();
}

void InstallPage::initializePage()
{
    setTitle("Ready For Installation");
}

bool InstallPage::validatePage()
{
    m_cabinet->setProgressHandler(this);

    m_cabinet->scanFile();
    m_cabinet->closeFile();

    m_cabinet->setProgressHandler(NULL);

    return true;
}

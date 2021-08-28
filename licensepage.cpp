#include "licensepage.h"
#include "datacabinet.h"

#include <qtgui>

LicensePage::LicensePage()
    : m_cabinet(0), m_ownLicense(true)
{
    setTitle("QtInstall Terms of Use");
    construct();
}
LicensePage::LicensePage(DataCabinet *cab)
    : m_cabinet(cab), m_ownLicense(false)
{
//    setTitle("QtInstall Terms of Use");
    construct();
}

void LicensePage::construct()
{
     m_edit = new QTextEdit();
     m_edit->setReadOnly(true);
     //m_edit->setFrameStyle(QFrame::NoFrame);

     m_check = new QCheckBox("I accept the license");

     QVBoxLayout *layout = new QVBoxLayout;
     layout->addWidget(m_edit);
     layout->addWidget(m_check);
     setLayout(layout);
}

void LicensePage::initializePage()
{
    QString licText;

    if( m_ownLicense )
    {
        QFile lic(":/documentation/GPL-license.html");
        if( lic.open(QIODevice::ReadOnly) )
        {
            QString lictext(lic.readAll());
            lictext.replace("%VERSION%",QtInstallVersion);
            m_edit->setText(lictext);
        }
        else
            m_edit->setText(QString("QtInstall version ") + QtInstallVersion +", Copyright (C) 2009 Valentin Illich.\n\nQtInstall comes with ABSOLUTELY NO WARRANTY; for details type `Help'. This is free software, and you are welcome to redistribute it under certain conditions.");
    }
    else
    {
        setTitle(m_cabinet->getProperty(ePropWindowTitle)+" License");
        m_edit->setText(m_cabinet->getProperty(ePropLicenceText));
    }

}

bool LicensePage::validatePage()
{
    if( m_check->isChecked() )
    {
        QSettings sett;

        sett.setValue("QtInstallVersion",QtInstallVersion);
        return true;
    }
    else
    {
        QMessageBox::warning(this,"license text","You must accept the license before you may continue!");
        return false;
    }
}

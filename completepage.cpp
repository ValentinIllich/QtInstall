#include "completepage.h"
#include "datacabinet.h"
#include "pathmanagement.h"

#include <qtgui>

#include "../utilities.h"

CompletePage::CompletePage(DataCabinet *cab)
    : m_cabinet(cab)
    , m_isMajorUpdate(false)
    , m_isMinorUpdate(false)
{
    cab->isUpdateInstallation(m_isMajorUpdate,m_isMinorUpdate);

    m_label = new QLabel();
    m_label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_label);
    setLayout(layout);
}

void CompletePage::initializePage()
{
    QString text = m_cabinet->getProperty(ePropCompletionText);

    if( m_cabinet->hasError() )
    {
        if( m_isMajorUpdate || m_isMinorUpdate )
            setTitle("Installation Failed!");
        else
            setTitle("Removing Software Failed!");

        m_label->setText("The installation failed for some reason. Please see the Log or contact the manufacturer of this package.");
        setButtonText(QWizard::CustomButton1,"Show Log");
        wizard()->setOptions( wizard()->options() | QWizard::HaveCustomButton1 );

        if( PathManagement::hasAdminAcces() )
            QMessageBox::critical(this,"installation error",QString("The installation failed for following reason:\n")
                                  +getErrorMessage()+"For details, please click on 'Show Log'.");
        else
            QMessageBox::critical(this,"installation error",QString("Not all parts of this package could be installed. ")
                                  +"Probably you will need to repeat the installation with administrator rights.\n"
                                  +"For details, please click on 'Show Log'.");
    }
    else
    {
        if( m_isMajorUpdate || m_isMinorUpdate )
        {
            setTitle("Installation Finished");

            if( text.isEmpty() )
                m_label->setText("the installation has succesfully been completed.");
            else
                m_label->setText(text);
        }
        else
        {
            setTitle("Removing Software Finished");
            m_label->setText("The installation has succesfully been removed.");
        }

    }
}

bool CompletePage::validatePage()
{
    return true;
}

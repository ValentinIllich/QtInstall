#include "completepage.h"
#include "datacabinet.h"
#include "pathmanagement.h"

#include <qtgui>

#include "../utilities.h"

CompletePage::CompletePage(DataCabinet *cab)
    : m_cabinet(cab)
{
    m_label = new QLabel("The installation has succesfully completed.");
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
        setTitle("Installation Failed!");

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
        setTitle("Installation Finished");

        if( text.isEmpty() )
            m_label->setText("the installation has succesfully finished.");
        else
            m_label->setText(text);
    }
}

bool CompletePage::validatePage()
{
    return true;
}

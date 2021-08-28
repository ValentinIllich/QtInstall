#include "welcomepage.h"
#include "datacabinet.h"

#include <qtgui>

WelcomePage::WelcomePage(DataCabinet *cab)
    : m_cabinet(cab)
{
//     m_label = new QLabel("");
//     m_label->setWordWrap(true);
//     m_label->setTextFormat(Qt::RichText);

     m_edit = new QTextEdit();
     m_edit->setReadOnly(true);
     m_edit->setFrameStyle(QFrame::NoFrame);

     QVBoxLayout *layout = new QVBoxLayout;
//     layout->addWidget(m_label);
     layout->addWidget(m_edit);
     setLayout(layout);
}

void WelcomePage::initializePage()
{
    setTitle("Welcome");

//    m_label->setText(m_cabinet->getProperty(ePropWelcomeText));
    m_edit->setText(m_cabinet->getProperty(ePropWelcomeText));
}

bool WelcomePage::validatePage()
{
    return true;
}

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

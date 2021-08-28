#include "welcomepage.h"
#include "datacabinet.h"

#include <QtGui>
#include <QVBoxLayout>

WelcomePage::WelcomePage(DataCabinet *cab)
    : m_cabinet(cab)
{
//     m_label = new QLabel("");
//     m_label->setWordWrap(true);
//     m_label->setTextFormat(Qt::RichText);

     m_edit = new QTextEdit();
     m_edit->setReadOnly(true);
//     m_edit->setFrameStyle(QFrame::NoFrame);

     QVBoxLayout *layout = new QVBoxLayout;
//     layout->addWidget(m_label);
     layout->addWidget(m_edit);
     setLayout(layout);
}

void WelcomePage::initializePage()
{
    setTitle("Welcome to "+m_cabinet->getProperty(ePropWindowTitle));

//    m_label->setText(m_cabinet->getProperty(ePropWelcomeText));
    m_edit->setText(m_cabinet->getProperty(ePropWelcomeText));
}

bool WelcomePage::validatePage()
{
    return true;
}

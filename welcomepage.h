#ifndef WELCOMEPAGE_H
#define WELCOMEPAGE_H

#include <QWizardPage>
#include <QLabel>
#include <QTextEdit>
#include <QCheckBox>

#include "datacabinet.h"

class WelcomePage : public QWizardPage
{
public:
    WelcomePage(DataCabinet *cab);

    virtual void initializePage();
    virtual bool validatePage();

private:
    DataCabinet *m_cabinet;

//    QLabel *m_label;
    QTextEdit *m_edit;
};

#endif // WELCOMEPAGE_H

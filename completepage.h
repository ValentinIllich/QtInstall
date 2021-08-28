#ifndef COMPLETEPAGE_H
#define COMPLETEPAGE_H

#include <QWizardPage>
#include <QLabel>
#include <QTextEdit>
#include <QCheckBox>

#include "datacabinet.h"

class CompletePage : public QWizardPage
{
public:
    CompletePage(DataCabinet *cab);

    virtual void initializePage();
    virtual bool validatePage();

private:
    DataCabinet *m_cabinet;

    QLabel *m_label;
};

#endif // COMPLETEPAGE_H

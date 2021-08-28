#ifndef LICENSEPAGE_H
#define LICENSEPAGE_H

#include <QWizardPage>
#include <QLabel>
#include <QTextEdit>
#include <QCheckBox>

#include "datacabinet.h"

class LicensePage : public QWizardPage
{
public:
    LicensePage();
    LicensePage(DataCabinet *cab);

    virtual void initializePage();
    virtual bool validatePage();

private:
    void construct();
    DataCabinet *m_cabinet;

    QTextEdit *m_edit;
    QCheckBox *m_check;

    bool m_ownLicense;
};

#endif // LICENSEPAGE_H

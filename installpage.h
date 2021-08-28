#ifndef INSTALLPAGE_H
#define INSTALLPAGE_H

#include <QWizardPage>
#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>
#include <QProgressBar>

#include "datacabinet.h"

class InstallPage : public QWizardPage, public IDataProgress
{
public:
    InstallPage(DataCabinet *cab);

    virtual void initializePage();
    virtual bool validatePage();

    // IDataProgress interface
    virtual void setProgress(int progress);

private:
    DataCabinet *m_cabinet;

    bool m_isRemoving;
    bool m_isMajorUpdate;
    bool m_isMinorUpdate;

    QLabel *m_label;
    QCheckBox *m_box;
    QRadioButton *m_rb1;
    QRadioButton *m_rb2;
    QProgressBar *m_bar;
};

#endif // INSTALLPAGE_H

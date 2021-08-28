#ifndef INSTALLPAGE_H
#define INSTALLPAGE_H

#include <QWizardPage>
#include <QLabel>
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
    QProgressBar *m_bar;
};

#endif // INSTALLPAGE_H

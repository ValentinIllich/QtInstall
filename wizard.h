#ifndef WIZARD_H
#define WIZARD_H

#include <QWizard>

#include "datacabinet.h"

/**
 this class is the main user interface for the QtInstall tool. It is inherited from QWizard and does all the jobs.
 */
class Wizard : public QWizard
{
    Q_OBJECT

public:
	/** constructor */
    Wizard(DataCabinet *cab,QWidget *parent = 0);
	/** destructor */
    ~Wizard();

public slots:
    void showHelp();
    void showCustom(int which);

private:
	/** reference to the data cabinet container class */
    DataCabinet *m_cabinet;
};

#endif // WIZARD_H

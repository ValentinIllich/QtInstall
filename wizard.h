#ifndef WIZARD_H
#define WIZARD_H

#include <qtgui>
//#include <QWizard>

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

	void setDebugMode(bool debugging);

public slots:
    void showHelp();
    void showCustom(int which);

protected:
    void closeEvent ( QCloseEvent * event );

private:
	/** reference to the data cabinet container class */
    DataCabinet *m_cabinet;
    /** when doing a major update (removing old installation befor eupdate) this is true */
    bool m_isMajorUpdate;
    /** when doing a minor update (just updating files) this is true */
    bool m_isMinorUpdate;
    /** when in debugging mode, delet on close, no actions */
	bool m_debugging;
};

class ConfigPage : public QWizardPage
{
    Q_OBJECT

public:
    ConfigPage();
	~ConfigPage();

public slots:
    void showPreview();
	void selectFile();
	void create();

    virtual void initializePage();
    virtual bool validatePage();

private:
    QLabel *m_filename;
	QPushButton *m_preview;
	QPushButton *m_fileselect;
	QPushButton *m_create;
    QCheckBox *m_deatailed;
    QPlainTextEdit *m_edit;
};

#endif // WIZARD_H

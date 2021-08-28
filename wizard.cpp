#include "wizard.h"

#include "welcomepage.h"
#include "installpage.h"

#include <QProcess>
#include <QFileInfo>
#include <QDir>

#if defined(Q_OS_WIN32)
//#include <tchar.h>
#include <windows.h>
#endif

Wizard::Wizard(DataCabinet *cab,QWidget *parent)
    : QWizard(parent)
    , m_cabinet(cab)
{
    setOption(QWizard::HaveHelpButton);

    addPage(new WelcomePage(m_cabinet));
    addPage(new InstallPage(m_cabinet));
    addPage(new CompletePage(m_cabinet));

    connect(this, SIGNAL(helpRequested()), this, SLOT(showHelp()));
    connect(this, SIGNAL(customButtonClicked(int)), this, SLOT(showCustom(int)));
}

Wizard::~Wizard()
{
}

/** this method will display a custom button */
void Wizard::showCustom(int which)
{
    switch( which )
    {
        case CustomButton1:
            dbgVisible(true);
            accept();
            break;
    }
}

/** this method shows the help */
void Wizard::showHelp()
{
    dbgout("help...");
    QString name;

    QFile::copy(":/documentation/QtInstall.pdf","QtInstall.pdf");

    QString webbrowser;
#if defined(Q_OS_WIN32)
	name = QString("file://")+QDir::currentPath()+"/QtInstall.pdf";
//        QT_WA({
//            ShellExecute(winId(), 0, (TCHAR*)name.utf16(), 0, 0, SW_SHOWNORMAL);
//        } , {
            ShellExecuteA(winId(), 0, name.toLocal8Bit(), 0, 0, SW_SHOWNORMAL);
//        });
#endif
#if defined(Q_OS_MAC)
        name = "QtInstall.pdf";
        webbrowser = "open";
#endif

    if( !webbrowser.isEmpty() )
    {
        QProcess *proc = new QProcess(this);
//		show.startDetached("notepad " + fn);
        QObject::connect(proc, SIGNAL(finished(int)), proc, SLOT(deleteLater()));

        QStringList args;
        if (webbrowser == QLatin1String("kfmclient"))
            args.append(QLatin1String("exec"));
        args.append(name);
        proc->start(webbrowser, args);
    }
}

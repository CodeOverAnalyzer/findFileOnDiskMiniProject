#include "mainwindow.h"
#include <QApplication>
#include <QtWidgets>
#include <QSplashScreen>
#include <QSettings>
#include <memory>
#include <QFontDatabase>
//#include <getopt.h>
#include <QApplication>
#include <QMessageBox>
#include <QWidget>
#include <QDir>
#include <QDebug>
#include <QObject>
#include <QLabel>
#include <QPainter>
#include <QTime>
#include <QElapsedTimer>
#include <cstdio>



#define APPNAME "WinlandSellerClientsApplication"

static const int LOAD_TIME_MSEC = 5 * 1000;

static const int PROGRESS_X_PX = 0;
static const int PROGRESS_Y_PX = 180;
static const int PROGRESS_WIDTH_PX = 520;
static const int PROGRESS_HEIGHT_PX = 20;

#include <QApplication>

int main(int argc, char *argv[])
{

    /**
     * @brief mainWindow
     */
    QLocale::setDefault(QLocale(QLocale::Persian, QLocale::Iran));
    int ret = 0;


#if defined(Q_OS_WIN32)

    QCoreApplication::addLibraryPath(".\\");
#endif

// TODO imple if we have to restart the application
#if defined(Q_OS_MAC)
    Application app(argc, argv);
#else
    QApplication app(argc, argv);
#endif


    QApplication::processEvents();

    QFile f(":/style.qss");
    if (!f.exists())
    {
        printf("Unable to set stylesheet, file not found\n");
    }
    else
    {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }


    //*****************************************************************


    MainWindow *mainWindow = new MainWindow();


    //mainWindow->setWindowState(Qt::WindowMaximized);
    mainWindow->show();

    return app.exec();
}

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QScrollArea>
#include <QtWidgets>

#include <filefinderwidget.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool maximized() const;
    void setMaximized(bool maximized);

    void showWindow();

public slots:
    void fullScreen(bool fullscreen);
    void quit();

private:
    Ui::MainWindow *ui;


    FileFinderWidget * fileFinderWidget;
    QToolBar *toolBar ;
    QStackedWidget *stackedWidget;


    bool mMaximized;

};
#endif // MAINWINDOW_H

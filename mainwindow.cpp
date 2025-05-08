#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    ui->setupUi(this);

    QScrollArea *mainScroll = new QScrollArea();
    QWidget     *mainScrollWidget = new QWidget();

    //    mainScroll->setStyleSheet("{ background: none; border:none }");
    //    mainScroll->setStyleSheet("QWidget {  border:none }");
    //    mainScrollWidget->setStyleSheet("{ background: none; border:none }");
    //    mainScrollWidget->setStyleSheet("QWidget {  border:none }");

    mainScroll->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    mainScroll->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    QHBoxLayout *mainScrollLayout = new QHBoxLayout;

    QWidget     *mainWidget  = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setAlignment(Qt::AlignCenter);

    setLayout(mainLayout);

    ////////////////////////////////////////////////////////
    /// \brief Hlayout
    ///

    QHBoxLayout *Hlayout = new QHBoxLayout;
    Hlayout->setAlignment(Qt::AlignTop);
    setLayout(Hlayout);

    QWidget *canvas = new QWidget;
    canvas->setStyleSheet(" font: 11pt;  font-weight:bold");
    Hlayout->addWidget(canvas);


    QSpacerItem *space = new QSpacerItem(4,2);

    Hlayout = new QHBoxLayout;
    canvas->setLayout(Hlayout);

    QWidget* empty = new QWidget();
    empty->setStyleSheet("background-color: none;");
    empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);

    QToolButton *fullScreenButton = new QToolButton();
    QAction *actionFullScreen = new QAction(QIcon(":/icons8_full_screen_32px.png"), QString(""));
    actionFullScreen->setShortcut(QKeySequence(tr("F11")));
    actionFullScreen->setCheckable(true);
    actionFullScreen->setChecked(false);
    fullScreenButton->setDefaultAction(actionFullScreen);

    QToolButton *exitButton = new QToolButton();
    QAction *actionExit = new QAction(QIcon(":/deletePress"), QString(""));
    actionExit->setShortcut(QKeySequence(tr("F2")));
    actionExit->setCheckable(true);
    actionExit->setChecked(false);
    exitButton->setDefaultAction(actionExit);

    Hlayout->addSpacerItem(space);

    Hlayout->addSpacerItem(space);

    //    Hlayout->addWidget(lblTime);
    //    Hlayout->addWidget(lblDate);
    //    Hlayout->addWidget(m_avatarAlarms);
    Hlayout->addSpacerItem(space);
    //    Hlayout->addWidget(m_avatarMessages);
    Hlayout->addSpacerItem(space);
    //Hlayout->addSpacerItem(QSP);

    Hlayout->addWidget(empty);

    Hlayout->addWidget(fullScreenButton);
    Hlayout->addWidget(exitButton);

    QWidget     *toolbarWidget  = new QWidget;
    toolbarWidget->setLayout(Hlayout);
    //////////////////////////////////////////////////////////
    ///

    toolBar = new QToolBar();

    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBar->setIconSize(QSize(32,32));

    stackedWidget = new QStackedWidget(this);

    fileFinderWidget = new FileFinderWidget;

    stackedWidget->addWidget(fileFinderWidget);



    mainScrollWidget->setLayout(mainScrollLayout);

    mainWidget->setLayout(mainLayout);

    mainLayout->setAlignment(Qt::AlignCenter);
    setCentralWidget(mainWidget);

    //    ui->toolBar->hide();
    //mainScrollLayout->addWidget(toolBar);
    mainLayout->addWidget(toolbarWidget);
    QLabel *lblWelcome = new QLabel;

    mainScrollLayout->addWidget(stackedWidget);

    //    stackedWidget->setMinimumHeight(2000);

    //    mainLayout->addWidget(mainScroll);
    mainLayout->addWidget(mainScrollWidget);

    connect(actionFullScreen, SIGNAL(toggled(bool)), this, SLOT(fullScreen(bool)));
    connect(actionExit, SIGNAL(triggered()), this, SLOT(quit()));


    // connect(ui->homeWidget, SIGNAL(showLivePage()), this, SLOT(showLivePage()));
    //    connect(ui->homeWidget, SIGNAL(showDesignPage()), this, SLOT(showDesignPage()));
    // connect(mainToolBarWidget, SIGNAL(fullScreen(bool)),this, SLOT(fullScreen(bool)));
    // connect(mainToolBarWidget, SIGNAL(exit()),this, SLOT(quit()));

    //mainToolBarWidget->setMinimumHeight(84);
    //mainToolBarWidget->setStyleSheet(" font: 13pt;  font-weight:bold");


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showWindow()
{
    showNormal();
    setWindowState(Qt::WindowMaximized);
    showMaximized();
    raise();
    activateWindow();
    // a hack with UIElement application
#ifdef Q_OS_MAC
    utils::mac::orderFrontRegardless(seafApplet->mainWindow()->winId());
#endif
}


void MainWindow::fullScreen(bool fullscreen)
{
#ifdef __linux__
    setWindowState(windowState() ^ Qt::WindowFullScreen);
    if(fullscreen) {
        mNormalGeometry = geometry();
        ui->toolBar->hide();
    } else {
        setGeometry(mNormalGeometry);
        ui->toolBar->show();
    }
#elif _WIN32
    if(fullscreen) {
        mMaximized = isMaximized();
        setVisible(false);
        showFullScreen();
        showMaximized();
        //        ui->toolBar->hide();
        setVisible(true);
    } else {
        setVisible(false);
        if (mMaximized)
            showMaximized();
        else
            showNormal();
        toolBar->show();
        setVisible(true);
    }
#endif

}

void MainWindow::quit()
{
    this->close();
    qApp->exit();
    QCoreApplication::exit(0);
}

#include "filefinderwidget.h"
#include "ui_filefinderwidget.h"

#include <QtWidgets>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <filesystem>
#include <mutex>
#include <QThreadPool>
#include <QtConcurrent/QtConcurrent>
#include <filesearchtask.h>

enum { absoluteFileNameRole = Qt::UserRole + 1 };
std::mutex cout_mutex;
QThreadPool* threadPool = QThreadPool::globalInstance();

static inline QString fileNameOfItem(const QTableWidgetItem *item)
{
    return item->data(absoluteFileNameRole).toString();
}

static inline void openFile(const QString &fileName)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}


FileFinderWidget::FileFinderWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FileFinderWidget)
{
    ui->setupUi(this);

    setWindowTitle(tr("search Files"));
    QLabel *lblWelcome = new QLabel;
    lblWelcome->setText("WelCome");

    QPushButton *browseButton = new QPushButton(tr("&Browse..."), this);
    connect(browseButton, &QAbstractButton::clicked, this, &FileFinderWidget::browse);
    findButton = new QPushButton(tr("&Find"), this);
    connect(findButton, &QAbstractButton::clicked, this, &FileFinderWidget::find);

    fileComboBox = createComboBox(tr("*"));
    connect(fileComboBox->lineEdit(), &QLineEdit::returnPressed,
            this, &FileFinderWidget::animateFindClick);
    textComboBox = createComboBox();
    connect(textComboBox->lineEdit(), &QLineEdit::returnPressed,
            this, &FileFinderWidget::animateFindClick);
    directoryComboBox = createComboBox(QDir::toNativeSeparators(QDir::currentPath()));
    connect(directoryComboBox->lineEdit(), &QLineEdit::returnPressed,
            this, &FileFinderWidget::animateFindClick);

    filesFoundLabel = new QLabel;

    createFilesTable();


    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addWidget(lblWelcome);
    mainLayout->addWidget(new QLabel(tr("Named:")), 0, 0);
    mainLayout->addWidget(fileComboBox, 0, 1, 1, 2);
    mainLayout->addWidget(new QLabel(tr("Containing text:")), 1, 0);
    mainLayout->addWidget(textComboBox, 1, 1, 1, 2);
    mainLayout->addWidget(new QLabel(tr("In directory:")), 2, 0);
    mainLayout->addWidget(directoryComboBox, 2, 1);
    mainLayout->addWidget(browseButton, 2, 2);
    mainLayout->addWidget(filesTable, 3, 0, 1, 3);
    mainLayout->addWidget(filesFoundLabel, 4, 0, 1, 2);
    mainLayout->addWidget(findButton, 4, 2);

    connect(new QShortcut(QKeySequence::Quit, this), &QShortcut::activated,
            qApp, &QApplication::quit);
}

FileFinderWidget::~FileFinderWidget()
{
    delete ui;
}

void FileFinderWidget::browse()
{
    QString directory =
        QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("Find Files"), QDir::currentPath()));

    if (!directory.isEmpty()) {
        if (directoryComboBox->findText(directory) == -1)
            directoryComboBox->addItem(directory);
        directoryComboBox->setCurrentIndex(directoryComboBox->findText(directory));
    }
}

static void updateComboBox(QComboBox *comboBox)
{
    if (comboBox->findText(comboBox->currentText()) == -1)
        comboBox->addItem(comboBox->currentText());
}

void searchFile(const QString& dirPath, const QString& fileName, QList<QString>& results) {
    QDir dir(dirPath);
    if (!dir.exists()) return;

    QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo& fileInfo : files) {
        if (fileInfo.isFile() && fileInfo.fileName() == fileName) {
            results.append(fileInfo.absoluteFilePath());
        } else if (fileInfo.isDir()) {
            searchFile(fileInfo.absoluteFilePath(), fileName, results);
        }
    }
}

void FileFinderWidget::startSearch(const QString& rootDir, const QString& fileName) {
    QList<QString> results;
    QFuture<void> future = QtConcurrent::run(searchFile, rootDir, fileName, std::ref(results));
    future.waitForFinished();

    // Process results
    for (const QString& filePath : results) {
        qDebug() << "Found file:" << filePath;
    }
}

void FileFinderWidget::find()
{
    filesTable->setRowCount(0);

    QString fileName = fileComboBox->currentText();
    QString text = textComboBox->currentText();
    QString path = QDir::cleanPath(directoryComboBox->currentText());
    currentDir = QDir(path);

    updateComboBox(fileComboBox);
    updateComboBox(textComboBox);
    updateComboBox(directoryComboBox);

    ///////////////////////////////////
    ///
    //QString fileName = "interview.pdf";
    QFileInfoList drives = QDir::drives();

    QMutex mutex;
    QQueue<QString> resultQueue;
    QList<FileSearcher*> searchThreads;

    for (const QFileInfo& drive : drives) {
        FileSearcher* searchThread = new FileSearcher(drive.fileName(), fileName, &mutex, &resultQueue);
        searchThreads.append(searchThread);
        searchThread->start();
    }

    for (FileSearcher* thread : searchThreads)
        thread->wait();

    for (FileSearcher* thread : searchThreads) {
        thread->stop();
        delete thread;
    }

    qDebug() << "Found files:";
    while (!resultQueue.isEmpty()) {
        qDebug() << resultQueue.dequeue();
        QString filePath = resultQueue.dequeue();
        const QString toolTip = QDir::toNativeSeparators(filePath);
        const QString relativePath = QDir::toNativeSeparators(currentDir.relativeFilePath(filePath));
        const qint64 size = QFileInfo(filePath).size();
        QTableWidgetItem *fileNameItem = new QTableWidgetItem(relativePath);
        fileNameItem->setData(absoluteFileNameRole, QVariant(filePath));
        fileNameItem->setToolTip(toolTip);
        fileNameItem->setFlags(fileNameItem->flags() ^ Qt::ItemIsEditable);
        QTableWidgetItem *sizeItem = new QTableWidgetItem(QLocale().formattedDataSize(size));
        sizeItem->setData(absoluteFileNameRole, QVariant(filePath));
        sizeItem->setToolTip(toolTip);
        sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        sizeItem->setFlags(sizeItem->flags() ^ Qt::ItemIsEditable);

        int row = filesTable->rowCount();
        filesTable->insertRow(row);
        filesTable->setItem(row, 0, fileNameItem);
        filesTable->setItem(row, 1, sizeItem);
    }
    filesFoundLabel->setText(tr("%n file(s) found (Double click on a file to open it)", nullptr, resultQueue.size()));
    filesFoundLabel->setWordWrap(true);



    ///
    /////////////////////////////////////////////////

    //startSearch(path,fileName);
    //std::vector<std::filesystem::path> search_roots;
    //search_roots.push_back("E:\\");

    //threaded_search(search_roots, "photo_۲۰۲۵-۰۴-۰۱_۲۱-۴۷-۲۸.jpg");

    //FileSearchTask* task = new FileSearchTask(path, fileName);
    //connect(task, &FileSearchTask::fileFound, this, &FileFinderWidget::handleFileFound);
    //threadPool->start(task);



    // QStringList filter;
    // if (!fileName.isEmpty())
    //     filter << fileName;
    // QDirIterator it(path, filter, QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    // QStringList files;
    // while (it.hasNext())
    //     files << it.next();
    // if (!text.isEmpty())
    //     files = findFiles(files, text);
    // files.sort();
    // showFiles(files);
}

void FileFinderWidget::handleFileFound(const QString& filePath) {
    // Process the found file path
    qDebug() << "Found file:" << filePath;
}

void FileFinderWidget::animateFindClick()
{
    findButton->animateClick();
}

void search_directory(const std::filesystem::path& dir_path, const std::string& filename) {
    try {
        for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
            if (entry.is_regular_file() && entry.path().filename() == filename) {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "Found file: " << entry.path().string() << std::endl;
            } else if (entry.is_directory()) {
                search_directory(entry.path(), filename);
            }
        }
    } catch (const std::exception& e) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cerr << "Error accessing " << dir_path << ": " << e.what() << std::endl;
    }
}

void FileFinderWidget::threaded_search(const std::vector<std::filesystem::path>& roots, const std::string& filename) {
    std::vector<std::thread> threads;
    for (const auto& root : roots) {
        threads.emplace_back(search_directory, root, filename);
    }

    for (auto& thread : threads) {
        thread.join();
    }
}


QStringList FileFinderWidget::findFiles(const QStringList &files, const QString &text)
{
    QProgressDialog progressDialog(this);
    progressDialog.setCancelButtonText(tr("&Cancel"));
    progressDialog.setRange(0, files.size());
    progressDialog.setWindowTitle(tr("Find Files"));

    QMimeDatabase mimeDatabase;
    QStringList foundFiles;

    for (int i = 0; i < files.size(); ++i) {
        progressDialog.setValue(i);
        progressDialog.setLabelText(tr("Searching file number %1 of %n...", nullptr, files.size()).arg(i));
        QCoreApplication::processEvents();

        if (progressDialog.wasCanceled())
            break;

        const QString fileName = files.at(i);
        const QMimeType mimeType = mimeDatabase.mimeTypeForFile(fileName);
        if (mimeType.isValid() && !mimeType.inherits(QStringLiteral("text/plain"))) {
            qWarning() << "Not searching binary file " << QDir::toNativeSeparators(fileName);
            continue;
        }
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QString line;
            QTextStream in(&file);
            while (!in.atEnd()) {
                if (progressDialog.wasCanceled())
                    break;
                line = in.readLine();
                if (line.contains(text, Qt::CaseInsensitive)) {
                    foundFiles << files[i];
                    break;
                }
            }
        }
    }
    return foundFiles;
}

void FileFinderWidget::showFiles(const QStringList &paths)
{
    for (const QString &filePath : paths) {
        const QString toolTip = QDir::toNativeSeparators(filePath);
        const QString relativePath = QDir::toNativeSeparators(currentDir.relativeFilePath(filePath));
        const qint64 size = QFileInfo(filePath).size();
        QTableWidgetItem *fileNameItem = new QTableWidgetItem(relativePath);
        fileNameItem->setData(absoluteFileNameRole, QVariant(filePath));
        fileNameItem->setToolTip(toolTip);
        fileNameItem->setFlags(fileNameItem->flags() ^ Qt::ItemIsEditable);
        QTableWidgetItem *sizeItem = new QTableWidgetItem(QLocale().formattedDataSize(size));
        sizeItem->setData(absoluteFileNameRole, QVariant(filePath));
        sizeItem->setToolTip(toolTip);
        sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        sizeItem->setFlags(sizeItem->flags() ^ Qt::ItemIsEditable);

        int row = filesTable->rowCount();
        filesTable->insertRow(row);
        filesTable->setItem(row, 0, fileNameItem);
        filesTable->setItem(row, 1, sizeItem);
    }
    filesFoundLabel->setText(tr("%n file(s) found (Double click on a file to open it)", nullptr, paths.size()));
    filesFoundLabel->setWordWrap(true);
}

QComboBox *FileFinderWidget::createComboBox(const QString &text)
{
    QComboBox *comboBox = new QComboBox;
    comboBox->setEditable(true);
    comboBox->addItem(text);
    comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return comboBox;
}

void FileFinderWidget::createFilesTable()
{
    filesTable = new QTableWidget(0, 2);
    filesTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList labels;
    labels << tr("Filename") << tr("Size");
    filesTable->setHorizontalHeaderLabels(labels);
    filesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    filesTable->verticalHeader()->hide();
    filesTable->setShowGrid(false);
    //! [15]
    filesTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(filesTable, &QTableWidget::customContextMenuRequested,
            this, &FileFinderWidget::contextMenu);
    connect(filesTable, &QTableWidget::cellActivated,
            this, &FileFinderWidget::openFileOfItem);
}


void FileFinderWidget::openFileOfItem(int row, int /* column */)
{
    const QTableWidgetItem *item = filesTable->item(row, 0);
    openFile(fileNameOfItem(item));
}


void FileFinderWidget::contextMenu(const QPoint &pos)
{
    const QTableWidgetItem *item = filesTable->itemAt(pos);
    if (!item)
        return;
    QMenu menu;
#ifndef QT_NO_CLIPBOARD
    QAction *copyAction = menu.addAction("Copy Name");
#endif
    QAction *openAction = menu.addAction("Open");
    QAction *action = menu.exec(filesTable->mapToGlobal(pos));
    if (!action)
        return;
    const QString fileName = fileNameOfItem(item);
    if (action == openAction)
        openFile(fileName);
#ifndef QT_NO_CLIPBOARD
    else if (action == copyAction)
        QGuiApplication::clipboard()->setText(QDir::toNativeSeparators(fileName));
#endif
}

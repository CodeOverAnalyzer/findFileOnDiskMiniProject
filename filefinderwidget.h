#ifndef FILEFINDERWIDGET_H
#define FILEFINDERWIDGET_H

#include <QWidget>
#include <QDir>
#include <QDirIterator>
#include <QObject>

QT_BEGIN_NAMESPACE
class QComboBox;
class QLabel;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;
QT_END_NAMESPACE


namespace Ui {
class FileFinderWidget;
}

class FileFinderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileFinderWidget(QWidget *parent = nullptr);
    ~FileFinderWidget();

signals:
    void fullScreen(bool fullScreen);
    void exit();


private slots:
    void browse();
    void find();
    void animateFindClick();
    void openFileOfItem(int row, int column);
    void contextMenu(const QPoint &pos);
    void handleFileFound(const QString& filePath);

private:

    QStringList findFiles(const QStringList &files, const QString &text);
    void threaded_search(const std::vector<std::filesystem::path>& roots, const std::string& filename);
    void startSearch(const QString& rootDir, const QString& fileName);
    void showFiles(const QStringList &paths);
    QComboBox *createComboBox(const QString &text = QString());
    void createFilesTable();

    QComboBox *fileComboBox;
    QComboBox *textComboBox;
    QComboBox *directoryComboBox;
    QLabel *filesFoundLabel;
    QPushButton *findButton;
    QTableWidget *filesTable;

    QDir currentDir;

    Ui::FileFinderWidget *ui;
};


#endif // FILEFINDERWIDGET_H

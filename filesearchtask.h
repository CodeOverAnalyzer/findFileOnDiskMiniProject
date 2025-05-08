#include <QCoreApplication>
#include <QDirIterator>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QDebug>

class FileSearcher : public QThread {
    Q_OBJECT
public:
    FileSearcher(const QString& dirPath, const QString& fileName, QMutex* mutex, QQueue<QString>* resultQueue)
        : dirPath(dirPath), fileName(fileName), mutex(mutex), resultQueue(resultQueue), stopSearch(false) {}

    void stop() {
        stopSearch = true;
    }

    void run() override {
        QDirIterator it(dirPath, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            if (stopSearch)
                break;

            QString path = it.next();
            if (QFileInfo(path).fileName() == fileName) {
                QMutexLocker locker(mutex);
                resultQueue->enqueue(path);
            }
        }
    }

private:
    QString dirPath;
    QString fileName;
    QMutex* mutex;
    QQueue<QString>* resultQueue;
    bool stopSearch;
};

#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class Network : public QObject
{
    Q_OBJECT
public:
    explicit Network(QObject *parent = nullptr);
    ~Network();

    void addToDownloadList(QString task);
    QString getFileInfo(QString task);
    void getFileInfo2(QString task);
    void startDownload(QString task, QString path);

signals:
    void process(QString task, qint64 byteTotal, double percent, double speed);
    void taskFinished(QString task);
    void fileInfo(QString task, QString fileName, qint64 fileSize);

public slots:
    void deleteTask(QString task);
    void stopTask(QString task);
    void startTask(QString task);

private slots:
    void readyToRead();
    void downloadError(QNetworkReply::NetworkError code);
    void downloadFinished();
    void downloadProgress(qint64 byteReceived, qint64 byteTotal);
    void gotFileInfo();

private:
    QStringList downloadList;
    QString currentTask;
    QString currentGettingTask;
    QString currentFileName;

    QNetworkAccessManager manager;
    QNetworkReply *reply;
    QFile file;
    QTime downloadTime;

    QString downloadPath;

    bool isDownloading;
    qint64 bytesTotal;
    qint64 currentFileSize;
    int timeElapsed, timeLast;
};

#endif // NETWORK_H

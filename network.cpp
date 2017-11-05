#include <QFileInfo>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QEventLoop>
#include <QDebug>

#include "network.h"

Network::Network(QObject *parent) : QObject(parent),
    isDownloading(false),
    timeElapsed(0), timeLast(0)
{

}

Network::~Network()
{

}

/* 添加任务到下载列表中 */
void Network::addToDownloadList(QString task)
{
    downloadList.append(task);
}

/* 通过url获取文件名 */
QString Network::getFileInfo(QString task)
{
    QUrl url = QUrl::fromEncoded(task.toLocal8Bit());
    QString path = url.path();
    QString fileName = QFileInfo(path).fileName();

    return fileName;
}

void Network::getFileInfo2(QString task)
{
    QNetworkRequest request;

    currentGettingTask = task;
    QUrl url = QUrl::fromEncoded(task.toLocal8Bit());

    QString path = url.path();
    currentFileName = QFileInfo(path).fileName();

    request.setUrl(url);

    reply = manager.head(request);

    connect(reply, SIGNAL(finished()), this, SLOT(gotFileInfo()));
}

void Network::startDownload(QString task, QString path)
{
    /* 如果任务正在下载，则返回 */
    if (isDownloading) {
        return ;
    }

    QUrl url = QUrl::fromEncoded(task.toLocal8Bit());
    QString address = url.path();

    QString fileName = path + "/" + QFileInfo(address).fileName();

    file.setFileName(fileName);
    currentFileSize = file.size();

    currentTask = task;
    downloadPath = path;

    if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qDebug() << "Download error, cannot open file.";
    }

    QNetworkRequest request(url);

    request.setRawHeader("User-Agent", "MyOwnBrowser 1.0");
    request.setRawHeader("Range", QString("bytes=%1-").arg(currentFileSize).toLatin1());
//    qDebug() << "bytes downloaded:" << currentFileSize << "B";
    if (!file.seek(currentFileSize)) {
        qDebug() << "File seek failed.";
        return ;
    }

    reply = manager.get(request);

    connect(reply, SIGNAL(readyRead()), this, SLOT(readyToRead()));
    connect(reply, SIGNAL(finished()),  this, SLOT(downloadFinished()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(downloadProgress(qint64,qint64)));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(downloadError(QNetworkReply::NetworkError)));


    downloadTime.start();

    isDownloading = true;
}

void Network::gotFileInfo()
{
    qint64 fileSize;

    if (reply->hasRawHeader(QString("Content-Length").toUtf8())) {
        fileSize = reply->rawHeader(QString("Content-Length").toUtf8()).toLongLong();
    }

    qDebug() << "File name: " << currentFileName << " size: " << fileSize;

    emit fileInfo(currentGettingTask, currentFileName, fileSize);
}

void Network::deleteTask(QString task)
{
    if (downloadList.isEmpty() && !downloadList.contains(task)) {
        return ;
    }

    if (task == currentTask) {
        stopTask(task);
    }

    downloadList.removeOne(task);
}

void Network::startTask(QString task)
{
    if (!downloadList.contains(task)) {
        downloadList.append(task);
    }

    startDownload(task, downloadPath);
}

void Network::stopTask(QString task)
{
    if (task != currentTask) {
        return ;
    }

    if (!isDownloading) {
        return ;
    }
    isDownloading = false;

    timeElapsed += timeLast;

    disconnect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
    disconnect(reply, SIGNAL(readyRead()), this, SLOT(readyToRead()));
    disconnect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(downloadError(QNetworkReply::NetworkError)));
    disconnect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));

    reply->abort();
    reply->deleteLater();

    file.close();
}

void Network::readyToRead()
{
    /* write data to file */
    file.write(reply->readAll());
}

void Network::downloadError(QNetworkReply::NetworkError code)
{
    qDebug() << "Failed: " << code;

    stopTask(currentTask);
}


void Network::downloadFinished()
{
    file.close();

    timeElapsed = 0;

    isDownloading = false;

    downloadList.removeOne(currentTask);

    if (reply->error()) {
        qDebug() << "Failed: " << reply->errorString();
        return ;
    }

    reply->deleteLater();

    emit taskFinished(currentTask);

    if (!downloadList.isEmpty()) {
        QString taskNext = downloadList.first();
        startDownload(taskNext, downloadPath);
    }
}

void Network::downloadProgress(qint64 bytesReceived, qint64 byteTotal)
{
    double speed = (bytesReceived * 1000.0) / (downloadTime.elapsed() + timeElapsed);
    timeLast  = downloadTime.elapsed();

//    qDebug() << "Time elapse:" << downloadTime.elapsed() + timeElapsed;

    double percent = (bytesReceived + currentFileSize) * 100.0 / (byteTotal + currentFileSize);
    if (percent >= 100.0) {
        downloadFinished();
        return ;
    }

    emit process(currentTask, byteTotal + currentFileSize, percent, speed);
}

#include <QListWidgetItem>
#include <QMenu>
#include <QClipboard>
#include <windows.h>

#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    this->setFixedSize(400, 700);

    addTaskDlg = new AddTaskDialog;
    network    = new Network;

    initSlots();
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::initSlots()
{
    connect(ui->addTaskButton, SIGNAL(clicked()), this, SLOT(showAddTaskDialog()));
    connect(addTaskDlg, SIGNAL(downloadSettings(QString,QStringList)),
            this, SLOT(handleTaskRequest(QString,QStringList)));
    connect(network, SIGNAL(taskFinished(QString)), this, SLOT(downloadFinished(QString)));
    connect(network, SIGNAL(fileInfo(QString,QString,qint64)), this, SLOT(taskInfo(QString,QString,qint64)));
    connect(network, SIGNAL(process(QString,qint64,double,double)),
            this, SLOT(showDownloadProgress(QString,qint64,double,double)));
    connect(this, SIGNAL(stopDownloadTask(QString)), network, SLOT(stopTask(QString)));
    connect(this, SIGNAL(startDownloadTask(QString)), network, SLOT(startTask(QString)));
    connect(this, SIGNAL(removeDownloadTask(QString)), network, SLOT(deleteTask(QString)));
}

void MainWidget::updateTaskList(QStringList taskList)
{
    QStringList repeat;

    foreach (QString task, taskList) {
        /* 跳过重复的任务 */
        if (taskListCurrent.contains(task)) {
            repeat.append(task);
            continue;
        }
        network->getFileInfo2(task);
//      showTask(task);
//        network->addToDownloadList(task);
//        if (taskListCurrent.isEmpty()) {
//            network->startDownload(task, downloadPath);
//        }
//        taskListCurrent.append(task);
    }

    if (!repeat.isEmpty()) {
        qDebug() << repeat << "已在任务队列中！";
    }
}

void MainWidget::showTask(QString task, qint64 size)
{
    QString fileName = network->getFileInfo(task);

    ListWidget *widget = new ListWidget();

    int index = fileName.lastIndexOf(".");
    QString fileType = fileName.mid(index + 1);

    if (fileType == "bat") {
        widget->setHead(":/image/BAT.ico");
    } else if (fileType == "zip") {
        widget->setHead(":/image/ZIP.ico");
    } else if (fileType == "doc" || fileType == "docx") {
        widget->setHead(":/image/DOC.ico");
    } else if (fileType == "mp4") {
        widget->setHead(":/image/MP4.ico");
    } else if (fileType == "pdf") {
        widget->setHead(":/image/PDF.ico");
    } else if (fileType == "rmvb") {
        widget->setHead(":/image/RMVB.ico");
    } else if (fileType == "txt") {
        widget->setHead(":/image/TXT.ico");
    } else {
        widget->setHead(":/image/File.ico");
    }

    widget->setFileName(fileName);
    widget->setFileSize(size);
    widget->setSpeed(QString("等待下载"));

    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(QSize(0,50));
    ui->taskListWidget->addItem(item);
    ui->taskListWidget->setItemWidget(item, widget);
    widgetList.append(widget);
}

void MainWidget::showAddTaskDialog()
{
    addTaskDlg->show();
}

void MainWidget::handleTaskRequest(QString path, QStringList urls)
{
    addTaskDlg->close();

    downloadPath = path;

    updateTaskList(urls);
}

void MainWidget::taskInfo(QString task, QString fileName, qint64 fileSize)
{
    showTask(fileName, fileSize);

    network->addToDownloadList(task);

    if (taskListCurrent.isEmpty()) {
        network->startDownload(task, downloadPath);
    }

    taskListCurrent.append(task);
}

void MainWidget::startCurrentTask()
{
    int index = ui->taskListWidget->currentRow();

    if (taskListCurrent.isEmpty()) {
        return ;
    }
    QString task = taskListCurrent.at(index);

    emit startDownloadTask(task);
}

void MainWidget::stopCurrentTask()
{
    int index = ui->taskListWidget->currentRow();

    if (taskListCurrent.isEmpty()) {
        return ;
    }
    QString task = taskListCurrent.at(index);
    emit stopDownloadTask(task);

    ListWidget *widget = widgetList.at(index);
    widget->setSpeed("任务暂停中。");
}

void MainWidget::removeCurrentTask()
{
   int index = ui->taskListWidget->currentRow();

   removeTask(index);
}

void MainWidget::restartDownload()
{
    int index = ui->taskListWidget->currentRow();

    if (taskListCurrent.isEmpty()) {
        return ;
    }

    ListWidget *widget = widgetList.at(index);
    /* 获取当前控件的文件 */
    QString filename   = widget->getFileName();
    QString file       = downloadPath + filename;

    QFile::remove(file);

    QString task = taskListCurrent.at(index);

    emit startDownloadTask(task);
}

void MainWidget::removeTask(int index)
{
    QListWidgetItem *item = ui->taskListWidget->takeItem(index);
    delete item;
    widgetList.removeAt(index);

    if (taskListCurrent.isEmpty()) {
        return ;
    }
    QString task = taskListCurrent.at(index);
    taskListCurrent.removeAt(index);

    emit removeDownloadTask(task);
}

void MainWidget::deleteCurrentFile()
{
    int index = ui->taskListWidget->currentRow();

    if (taskListCurrent.isEmpty()) {
        return ;
    }

    ListWidget *widget = widgetList.at(index);
    /* 获取当前控件的文件 */
    QString filename   = widget->getFileName();
    QString file       = downloadPath + filename;

    removeTask(index);
    QFile::remove(file);
}

void MainWidget::copyToClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();

    int index = ui->taskListWidget->currentRow();

    if (taskListCurrent.isEmpty()) {
        return ;
    }
    QString address = taskListCurrent.at(index);

    /* 复制下载地址到系统剪切板 */
    clipboard->setText(address);
}

void MainWidget::showDownloadProgress(QString task, qint64 byteTotal, double percent, double speed)
{
    QString unit;

    if (speed < 1024.0) {
        unit = "B/s";
    } else if (speed < (1024 * 1024)) {
        speed /= 1024.0;
        unit = "kB/s";
    } else {
        speed /= 1024 * 1024;
        unit = "MB/s";
    }

    int index = taskListCurrent.indexOf(task);
    ListWidget *widget = widgetList.at(index);

    QString info = QString("完成进度: %1%, 当前速度: %2%3")
            .arg(percent, 2, 'f', 2)
            .arg(speed, 3, 'f', 2).arg(unit);

    widget->setFileSize(byteTotal);
    widget->setSpeed(info);
}

void MainWidget::downloadFinished(QString task)
{
    /* 获取当前任务的编号 */
    int index = taskListCurrent.indexOf(task);

    ListWidget *widget = widgetList.at(index);
    widget->setSpeed(QString("下载完成"));
}

void MainWidget::on_taskListWidget_customContextMenuRequested(const QPoint &pos)
{
    /* 判断右击处是否有item存在 */
    QListWidgetItem *item = ui->taskListWidget->itemAt(pos);
    if (item == NULL) {
        return;
    }

    QMenu *menu = new QMenu;

    QAction *actionStartTask = new QAction("开始任务", this);
    QAction *actionStopTask  = new QAction("暂停任务", this);
    QAction *actionDelTask   = new QAction("删除任务", this);
    QAction *actionRestartDownload   = new QAction("重新下载任务", this);
    QAction *actionDelFile   = new QAction("彻底删除任务", this);
    QAction *actionCopy      = new QAction("复制下载链接", this);

    connect(actionStartTask, SIGNAL(triggered(bool)), this, SLOT(startCurrentTask()));
    connect(actionStopTask,  SIGNAL(triggered(bool)), this, SLOT(stopCurrentTask()));
    connect(actionDelTask,   SIGNAL(triggered(bool)), this, SLOT(removeCurrentTask()));
    connect(actionRestartDownload,   SIGNAL(triggered(bool)), this, SLOT(restartDownload()));
    connect(actionDelFile,   SIGNAL(triggered(bool)), this, SLOT(deleteCurrentFile()));
    connect(actionCopy,      SIGNAL(triggered(bool)), this, SLOT(copyToClipboard()));

    menu->addAction(actionStartTask);
    menu->addAction(actionStopTask);
    menu->addAction(actionDelTask);
    menu->addAction(actionRestartDownload);
    menu->addAction(actionDelFile);
    menu->addAction(actionCopy);

    /* 在鼠标位置显示菜单 */
    menu->exec(QCursor::pos());

    /* 断开信号-槽 */
    disconnect(actionStartTask, SIGNAL(triggered(bool)), this, SLOT(startCurrentTask()));
    disconnect(actionStopTask,  SIGNAL(triggered(bool)), this, SLOT(stopCurrentTask()));
    disconnect(actionDelTask,   SIGNAL(triggered(bool)), this, SLOT(removeCurrentTask()));
    disconnect(actionRestartDownload,   SIGNAL(triggered(bool)), this, SLOT(restartDownload()));
    disconnect(actionDelFile,   SIGNAL(triggered(bool)), this, SLOT(deleteCurrentFile()));
    disconnect(actionCopy,      SIGNAL(triggered(bool)), this, SLOT(copyToClipboard()));

    /* 释放资源 */
    delete actionStartTask;
    delete actionStopTask;
    delete actionDelTask;
    delete actionRestartDownload;
    delete actionDelFile;
    delete actionCopy;
    delete menu;
}

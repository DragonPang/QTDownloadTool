#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

#include "addtaskdialog.h"
#include "listwidget.h"
#include "network.h"

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

private:
    void initSlots();
    void showTask(QString task, qint64 size = 0);
    void updateTaskList(QStringList taskList);
    void removeTask(int index);

signals:
    void removeDownloadTask(QString task);
    void stopDownloadTask(QString task);
    void startDownloadTask(QString task);

private slots:
    void showAddTaskDialog();
    void handleTaskRequest(QString path, QStringList urls);
    void showDownloadProgress(QString task, qint64 byteTotal, double percent, double speed);
    void downloadFinished(QString task);
    void startCurrentTask();
    void restartDownload();
    void stopCurrentTask();
    void removeCurrentTask();
    void copyToClipboard();
    void deleteCurrentFile();
    void taskInfo(QString task, QString fileName, qint64 fileSize);

    void on_taskListWidget_customContextMenuRequested(const QPoint &pos);

private:
    Ui::MainWidget *ui;

    AddTaskDialog *addTaskDlg;
    Network *network;

    QString downloadPath;
    QStringList taskListCurrent;
    QList<ListWidget *> widgetList;
};

#endif // MAINWIDGET_H

#ifndef ADDTASKDIALOG_H
#define ADDTASKDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class AddTaskDialog;
}

class AddTaskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddTaskDialog(QWidget *parent = 0);
    ~AddTaskDialog();

private:
    void initDownloadPath();
    void initSlots();
    QString getPath();
    void savePath(QString path);
    QStringList filterRepeatedUrls(QStringList list);

private slots:
    void startDownload();
    void choosePath();

signals:
    void downloadSettings(QString path, QStringList urls);

private:
    Ui::AddTaskDialog *ui;

    QSettings *settings;

    QString downloadPath;

};

#endif // ADDTASKDIALOG_H

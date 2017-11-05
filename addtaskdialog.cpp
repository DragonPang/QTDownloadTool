#include <QFileDialog>
#include <QDebug>

#include "addtaskdialog.h"
#include "ui_addtaskdialog.h"

AddTaskDialog::AddTaskDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddTaskDialog)
{
    ui->setupUi(this);

    this->setFixedSize(440, 350);

    initDownloadPath();
    initSlots();
}

AddTaskDialog::~AddTaskDialog()
{
    delete ui;
}

void AddTaskDialog::initDownloadPath()
{
    settings = new QSettings("settings.ini", QSettings::IniFormat);

    downloadPath = getPath();
    if (downloadPath == NULL) {
        downloadPath = "C:";
    }

    ui->pathLineEdit->setText(downloadPath);
}

void AddTaskDialog::initSlots()
{
    connect(ui->pathChooseButton, SIGNAL(clicked()), this, SLOT(choosePath()));
    connect(ui->startDownloadButton, SIGNAL(clicked()), this, SLOT(startDownload()));
}

void AddTaskDialog::savePath(QString path)
{
    downloadPath = path;

    settings->beginGroup("DownloadSettings");
    settings->setValue("path", QVariant(path));
    settings->endGroup();
}

QString AddTaskDialog::getPath()
{
    return settings->value("DownloadSettings/path").toString();
}

QStringList AddTaskDialog::filterRepeatedUrls(QStringList list)
{
    QStringList temp;

    foreach (QString str, list) {
        if (!temp.contains(str)) {
            temp.append(str);
        }
    }

    return temp;
}

void AddTaskDialog::choosePath()
{
    QFileDialog dialog;

    dialog.setDirectoryUrl(QUrl::fromLocalFile(downloadPath));
    dialog.setWindowTitle(tr("选择下载路径"));
    dialog.setFileMode(QFileDialog::Directory);
    if (dialog.exec()) {
        QString currentPath = dialog.selectedFiles()[0];
        if (currentPath == downloadPath) {
            return ;
        }

        ui->pathLineEdit->setText(currentPath);
        savePath(currentPath);
    }
}

void AddTaskDialog::startDownload()
{
    QString pathCurrent = ui->pathLineEdit->text();

    if (ui->urlTextEdit->toPlainText().isEmpty()) {
        qDebug() << "No task.";
        return ;
    }

    if (pathCurrent != downloadPath) {
        savePath(pathCurrent);
    }

    QStringList temp = ui->urlTextEdit->toPlainText().split(QRegExp(";|\\r|\\n"), QString::SkipEmptyParts);

    QStringList urls = filterRepeatedUrls(temp);

    ui->urlTextEdit->clear();

    emit downloadSettings(downloadPath, urls);
}


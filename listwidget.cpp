#include <QVBoxLayout>

#include "listwidget.h"
#include "ui_listwidget.h"

ListWidget::ListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ListWidget)
{
    ui->setupUi(this);

    init();
}

ListWidget::~ListWidget()
{
    delete ui;
}


void ListWidget::init()
{
    QVBoxLayout *vLayout = new QVBoxLayout;
    QHBoxLayout *hLayout = new QHBoxLayout;

    head = new QLabel(this);
    fileName = new QLabel(this);
    totalBytes = new QLabel(this);
    downloadSpeed  = new QLabel(this);

    ui->horizontalLayout->addWidget(head);

    hLayout->addWidget(fileName);
    hLayout->addWidget(totalBytes);

    vLayout->addLayout(hLayout);
    vLayout->addWidget(downloadSpeed);

    ui->horizontalLayout->addLayout(vLayout);
}

void ListWidget::setHead(QString path)
{
    QImage *image = new QImage(path);

    *image = image->scaled(36, 36, Qt::KeepAspectRatio);

    head->setFixedSize(36, 36);

    head->setPixmap(QPixmap::fromImage(*image));
}

void ListWidget::setFileName(QString name)
{
    fileName->setText(name);
}

void ListWidget::setFileSize(qint64 size)
{
    totalBytes->setText(QString("%1 B").arg(size));
}

void ListWidget::setSpeed(QString speed)
{
    downloadSpeed->setText(speed);
}

QString ListWidget::getFileName()
{
    return fileName->text();
}

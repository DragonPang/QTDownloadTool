#ifndef LISTWIDGETITEM_H
#define LISTWIDGETITEM_H

#include <QWidget>
#include <QLabel>

namespace Ui {
class ListWidget;
}

class ListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ListWidget(QWidget *parent = 0);
    ~ListWidget();

    void setHead(QString path);
    void setFileName(QString name);
    void setFileSize(qint64 size);
    void setSpeed(QString speed);

    QString getFileName();

private:
    void init();

private:
    Ui::ListWidget *ui;

    QLabel *head;
    QLabel *fileName;
    QLabel *totalBytes;
    QLabel *downloadSpeed;
};

#endif // LISTWIDGETITEM_H

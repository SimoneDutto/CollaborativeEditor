#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include "socket.h"
#include "uri.h"

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(Socket *sock = nullptr, QWidget *parent = nullptr);
    ~Dialog();

private slots:
    void on_pushButton_clicked();

private:
    Ui::Dialog *ui;
    Socket *socket;
   // MainWindow *mainwindow;

signals:
    void openThisFile(QString fileName);
    void destroyM(QString);
};

#endif // DIALOG_H

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

    void on_pushButton_2_clicked();
    void uriIsOk(QString uri);
    void uriIsNotOk();

private:
    Ui::Dialog *ui;
    Socket *socket;

signals:
    void openThisFile(QString fileName);
    void checkUri(QString uri);
};

#endif // DIALOG_H

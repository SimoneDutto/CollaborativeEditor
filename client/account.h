#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include "socket.h"
#include <QPixmap>
#include <QFile>

namespace Ui {
class Account;
}

class Account : public QDialog
{
    Q_OBJECT

public:
    explicit Account(Socket *sock = nullptr, QWidget *parent = nullptr, QString name = "");
    ~Account();

private slots:
    void on_pushButton_clicked();
    void on_setImage_clicked();

signals:
    void updateInfoClient(QString psw, QString path);
    void updateClientImageOnMainWindow(QString path);

private:
    Ui::Account *ui;
    Socket *socket;
    QString name;
    QString pathUserImage;
    QString oldImage;
    bool imageChanged;
    bool saveChanges;

    void closeEvent(QCloseEvent *event);

};

#endif // ACCOUNT_H

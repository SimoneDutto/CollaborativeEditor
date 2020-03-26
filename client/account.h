#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QDialog>
#include "socket.h"
#include <QPixmap>

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

private:
    Ui::Account *ui;
    Socket *socket;
    QString name;


};

#endif // ACCOUNT_H

#ifndef WELCOME_H
#define WELCOME_H

#include <QDialog>
#include "login.h"
#include "signup.h"
#include "socket.h"

namespace Ui {
class Welcome;
}

class Welcome : public QDialog
{
    Q_OBJECT

public:
    explicit Welcome(Socket *sock = nullptr, QWidget *parent = nullptr);
    ~Welcome();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::Welcome *ui;
    Login *login;
    SignUp *signup;
    Socket *sock;
};

#endif // WELCOME_H

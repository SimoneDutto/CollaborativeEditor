#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>
//#include <QCryptographicHash>
#include "mainwindow.h"
#include "newopen.h"
#include "socket.h"
#include "signup.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Login; }
QT_END_NAMESPACE

class Login : public QMainWindow
{
    Q_OBJECT

public:
    Login(Socket *sock = nullptr, QWidget *parent = nullptr);
    ~Login();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

public slots:
    void resumeLogin();
    void redoLogin();

    void notConnected();

private:
    Ui::Login *ui;
    MainWindow *mainWindow;
    Socket *socket;
    NewOpen *newopen;
    SignUp *signup;
};
#endif // LOGIN_H

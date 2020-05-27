#ifndef SIGNUP_H
#define SIGNUP_H

#include <QMainWindow>
#include "mainwindow.h"
#include "socket.h"
#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SignUp; }
QT_END_NAMESPACE

class SignUp : public QDialog
{
    Q_OBJECT

private:
    Ui::SignUp *ui;
    Socket *socket;
    MainWindow *mainWindow;
    QString pathUserImage;

public:
    explicit SignUp(Socket *sock = nullptr, QWidget *parent = nullptr);
    ~SignUp();

private slots:
    void on_pushButton_clicked();
    void on_setImage_clicked();
    void on_lineEdit_textEdited(const QString &arg1);
    void on_discardImage_clicked();
    void on_pushButton_2_clicked();

public slots:
    void sendToLogin();
    void repeatSignUp();
    void changeUsername();
};

#endif // SIGNUP_H

#include "login.h"
#include "ui_login.h"
#include <QMessageBox>

Login::Login(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Login)
{
    ui->setupUi(this);
}

Login::~Login()
{
    delete ui;
}


void Login::on_pushButton_clicked()
{
    QString username = ui->lineEdit_username->text();
    QString password = ui->lineEdit_password->text();

    //SEND username and password to server
    //WAIT server answer: CORRECT or NOT CORRECT

    if(username == "Vito" && password == "ciao2000"){
        //QMessageBox::information(this, "Login", "Username and password are correct");
        hide();
        mainWindow = new MainWindow(this);
        mainWindow->show();
    }
    else {
        QMessageBox::warning(this, "Error Login", "Username or password not correct");
    }
}

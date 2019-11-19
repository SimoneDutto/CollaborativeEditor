#include "login.h"
#include "ui_login.h"
#include <QMessageBox>

Login::Login(Socket *sock, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Login)
    , socket(sock)
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
    QByteArray password = ui->lineEdit_password->text().toLatin1();
    //QString hash_me = QString(QCryptographicHash::hash((password),QCryptographicHash::Md5).toHex());

    connect(socket, SIGNAL(loginSuccess()), this, SLOT(resumeLogin()));
    connect(socket, SIGNAL(loginError()), this, SLOT(redoLogin()));

    socket->sendLogin(username, password);
}

void Login::resumeLogin()
{
    newopen = new NewOpen(this->socket, this->socket->getFHandler(), this);
    //mainWindow = new MainWindow(this->socket, this->socket->getFHandler(), this);
    hide();
    //mainWindow->show();
    newopen ->show();
}

void Login::redoLogin()
{
    QMessageBox::warning(this, "Login Failed", "Retry");
}

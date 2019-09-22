#include "login.h"
#include "ui_login.h"
#include <QMessageBox>

Login::Login(Socket *sock, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Login)
    , socket(sock)
{
    ui->setupUi(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(socketReadyReadListFile()));
}

Login::~Login()
{
    delete ui;
}


void Login::on_pushButton_clicked()
{
    QString username = ui->lineEdit_username->text();
    QByteArray password = ui->lineEdit_password->text().toLatin1();
    QString hash_me = QString(QCryptographicHash::hash((password),QCryptographicHash::Md5).toHex());

    connect(socket, SIGNAL(loginSuccess()), this, SLOT(resumeLogin()));
    connect(socket, SIGNAL(loginError()), this, SLOT(redoLogin()));

    socket->checkLogin(username, hash_me);
}

void Login::resumeLogin()
{
    QMessageBox::warning(this, "Error Login", "Username or password not correct");
}

void Login::redoLogin()
{
    hide();
    mainWindow = new MainWindow(this->socket, this);
    mainWindow->show();
}

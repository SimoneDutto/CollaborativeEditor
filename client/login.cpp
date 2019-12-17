#include "login.h"
#include "ui_login.h"
#include <QMessageBox>
#include <QShortcut>

Login::Login(Socket *sock, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Login)
    , socket(sock)
{
    ui->setupUi(this);
    QPalette pal = palette();

    // set black background
    pal.setColor(QPalette::Background, QColor(128,128,128));
    pal.setColor(QPalette::WindowText, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->show();
    QShortcut *sc = new QShortcut(QKeySequence("Return"),ui->LoginBox);
    connect(sc, SIGNAL(activated()), ui->pushButton, SLOT(click()));
    connect(socket, SIGNAL(loginSuccess()), this, SLOT(resumeLogin()));
    connect(socket, SIGNAL(loginError()), this, SLOT(redoLogin()));
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

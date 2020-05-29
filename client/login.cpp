#include "login.h"
#include "ui_login.h"
#include <QMessageBox>
#include <QShortcut>
#include <QTimer>
#include "signup.h"
#include "serverdisc.h"

Login::Login(Socket *sock, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Login)
    , socket(sock)
{
    ui->setupUi(this);
    QPalette pal = palette();

    // set black background
    pal.setColor(QPalette::Background, QColor(58,58,60));
    pal.setColor(QPalette::WindowText, Qt::white);
    pal.setColor(QPalette::Button, QColor(229,229,234));
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    QPalette p = ui->lineEdit_password->palette();
    p.setColor(QPalette::Base, QColor(209,209,214));
    p.setColor(QPalette::Shadow, QColor(172,228,255));
    p.setColor(QPalette::Text, Qt::black);
    ui->lineEdit_username->setPalette(p);
    ui->lineEdit_password->setPalette(p);
    this->show();
    QTimer::singleShot(0, ui->lineEdit_username, SLOT(setFocus()));
    QShortcut *sc = new QShortcut(QKeySequence("Return"),ui->LoginBox);
    connect(sc, SIGNAL(activated()), ui->pushButton, SLOT(click()));
    connect(sock, SIGNAL(noConnection()), this, SLOT(notConnected()));
    connect(socket, SIGNAL(loginSuccess()), this, SLOT(resumeLogin()));
    connect(socket, SIGNAL(loginError()), this, SLOT(redoLogin()));
    if(!sock->getConnection()) emit sock->noConnection();
    setWindowTitle("");
    if(!sock->isConnected())
    {
        emit sock->noConnection();
    }
    ui->lineEdit_username->setStyleSheet(":focus {border: 2px solid #ace4c6};");
    ui->lineEdit_username->setAttribute(Qt::WA_MacShowFocusRect,0);
    ui->lineEdit_password->setStyleSheet(":focus {border: 2px solid #ace4c6};");
    ui->lineEdit_password->setAttribute(Qt::WA_MacShowFocusRect,0);
    ui->pushButton->setStyleSheet("color: white; background-color: #706d82;  border-radius:5px");
    ui->pushButton_2->setStyleSheet("color: white; background-color: #706d82;  border-radius:5px");
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
    if(QString::compare(ui->lineEdit_username->text(),"")==0 ||
            QString::compare(ui->lineEdit_password->text().toLatin1(),"")==0)
        QMessageBox::warning(this, "Ops...", "Insert username/password");

    else socket->sendLogin(username, password);
}

void Login::resumeLogin()
{
    this->close();
    newopen = new NewOpen(this->socket, this->socket->getFHandler(), this);
    //mainWindow = new MainWindow(this->socket, this->socket->getFHandler(), this);
    //mainWindow->show();
    newopen ->show();
}

void Login::redoLogin()
{
    QMessageBox::warning(this, "Login Failed", "Retry");
}

void Login::on_pushButton_2_clicked()
{
    this->close();
    signup = new SignUp(socket);
    signup->setAttribute(Qt::WA_DeleteOnClose, true);
    signup->show();
}

void Login::notConnected(){
    serverDisc *s = new serverDisc(this);
    s->show();
}


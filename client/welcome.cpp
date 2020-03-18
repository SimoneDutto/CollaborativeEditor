#include "welcome.h"
#include "ui_welcome.h"
#include "serverdisc.h"

Welcome::Welcome(Socket *sock, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Welcome),
    sock(sock)
{
    ui->setupUi(this);
    QPalette pal = palette();
    this->setWindowFlags(Qt::Window);
    connect(sock, SIGNAL(noConnection()), this, SLOT(notConnected()));
    // set black background
    pal.setColor(QPalette::Background, QColor(58,58,60));
    pal.setColor(QPalette::WindowText, Qt::white);
    pal.setColor(QPalette::Button, QColor(229,229,234));
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->show();
    setWindowTitle("");
    if(!sock->isConnected())
    {
        emit sock->noConnection();
    }
}

Welcome::~Welcome()
{
    delete ui;
}

void Welcome::on_pushButton_clicked()
{
    login = new Login(sock, this);
    hide();
    login->show();
}

void Welcome::on_pushButton_2_clicked()
{
    signup = new SignUp(sock, this);
    hide();
    signup->show();
}

void Welcome::notConnected(){
    serverDisc *s = new serverDisc(this);
    hide();
    s->show();
}

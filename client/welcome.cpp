#include "welcome.h"
#include "ui_welcome.h"

Welcome::Welcome(Socket *sock, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Welcome),
    sock(sock)
{
    ui->setupUi(this);
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

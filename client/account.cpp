#include "account.h"
#include "ui_account.h"

Account::Account(Socket *sock, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Account),
    socket(sock)
{
    ui->setupUi(this);
    ui->setupUi(this);
    QPalette pal = palette();

    // set black background
    pal.setColor(QPalette::Background, QColor(128,128,128));
    pal.setColor(QPalette::WindowText, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->show();

    // set picture
    QPixmap pix("path -- TO DO");
    ui->label->setPixmap(pix);
}

Account::~Account()
{
    delete ui;
}

void Account::on_pushButton_clicked()
{
    // salvare i cambiamenti nel db
}

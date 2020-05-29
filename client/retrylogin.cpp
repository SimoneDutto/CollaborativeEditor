#include "retrylogin.h"
#include "ui_retrylogin.h"

retryLogin::retryLogin(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::retryLogin)
{
    ui->setupUi(this);
    setWindowTitle("");
    setWindowModality(Qt::WindowModal);
    ui->pushButton->setStyleSheet("color: white; background-color: #706d82;  border-radius:5px");
}

retryLogin::~retryLogin()
{
    delete ui;
}

void retryLogin::on_pushButton_clicked()
{
    this->close();
}

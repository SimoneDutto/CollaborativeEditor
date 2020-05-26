#include "error.h"
#include "ui_error.h"

Error::Error(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Error)
{
    ui->setupUi(this);
    setWindowTitle("Error");
    setWindowModality(Qt::WindowModal);
    ui->pushButton->setStyleSheet("color: white; background-color: #706d82;  border-radius:5px");
}

Error::~Error()
{
    delete ui;
}

void Error::on_pushButton_clicked()
{
    this->close();
}

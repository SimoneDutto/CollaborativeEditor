#include "error.h"
#include "ui_error.h"

Error::Error(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Error)
{
    ui->setupUi(this);
    setWindowTitle("Error");
    this->setWindowFlags(Qt::Window);
}

Error::~Error()
{
    delete ui;
}

void Error::on_pushButton_clicked()
{
    this->close();
}

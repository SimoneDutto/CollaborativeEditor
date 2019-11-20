#include "form.h"
#include "ui_form.h"

Form::Form(QWidget *parent, Socket *s) :
    QDialog(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    connect( this, SIGNAL(newFile(QString)),
             s, SLOT(sendNewFile(QString)));
}

Form::~Form()
{
    delete ui;
}

void Form::on_pushButton_clicked()
{
    QString filename = ui->lineEdit->text();
    emit newFile(filename);
}

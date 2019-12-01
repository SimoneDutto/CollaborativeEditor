#include "form.h"
#include "ui_form.h"
#include <QShortcut>

Form::Form(Socket *sock, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Form),
    socket(sock)
{
    ui->setupUi(this);
    QShortcut *sc = new QShortcut(QKeySequence("Return"),this);
    connect(sc, SIGNAL(activated()), ui->pushButton, SLOT(click()));
    connect(this, SIGNAL(newFile(QString)), socket, SLOT(sendNewFile(QString)));
}

Form::~Form()
{
    delete ui;
}

void Form::on_pushButton_clicked()
{
    QString filename = ui->lineEdit->text();
    emit newFile(filename);
    qDebug() << "Inviata la richiesta a socket.cpp di creazione nuovo file: " << filename;
    hide();
}

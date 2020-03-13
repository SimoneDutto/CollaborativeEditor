#include "form.h"
#include "ui_form.h"
#include <QShortcut>

Form::Form(Socket *sock, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Form),
    socket(sock)
{
    ui->setupUi(this);
    QPalette pal = palette();

    // set black background
    pal.setColor(QPalette::Background, QColor(58,58,60));
    pal.setColor(QPalette::WindowText, Qt::white);
    pal.setColor(QPalette::Button, QColor(229,229,234));
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    QPalette p = ui->lineEdit->palette();
    p.setColor(QPalette::Base, QColor(209,209,214));
    p.setColor(QPalette::Text, Qt::black);
    ui->lineEdit->setPalette(p);
    setWindowTitle("New File");
    this->show();
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

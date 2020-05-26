#include "form.h"
#include "ui_form.h"
#include <QShortcut>
#include "error.h"

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
    ui->lineEdit->setStyleSheet(":focus {border: 2px solid #ace4c6};");
    ui->lineEdit->setAttribute(Qt::WA_MacShowFocusRect,0);

    QIcon *user_icon= new QIcon(":/rec/icone/icons8-aggiungi-file-96.png");
    ui->pushButton_2->setIcon(*user_icon);
    ui->pushButton_2->setIconSize(QSize(30, 30));
    QString styleSheet = "QPushButton {background-color: transparent; border-style: none; color: white}";
    ui->pushButton_2->setStyleSheet(styleSheet);
    ui->pushButton->setStyleSheet("color: white; background-color: #706d82;  border-radius:5px");
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
    if(filename.isEmpty()){
        Error *e = new Error(this);
        e->show();
    }
    else{
        emit newFile(filename);
        qDebug() << "Inviata la richiesta a socket.cpp di creazione nuovo file: " << filename;
        hide();
    }

}

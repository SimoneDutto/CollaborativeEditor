#include "account.h"
#include "ui_account.h"
#include "mainwindow.h"

Account::Account(Socket *sock, QWidget *parent, QString name) :
    QDialog(parent),
    ui(new Ui::Account),
    socket(sock),
    name(name)
{
    ui->setupUi(this);
    setWindowTitle("Edit Profile");
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
    ui->lineEdit_2->setPalette(p);
    ui->lineEdit_3->setPalette(p);
    ui->lineEdit_4->setPalette(p);

    /* Imposto la User Image */
    QString imageName = QString::number(socket->getClientID())+".png";
    QPixmap userPixmap = QPixmap(imageName);

    if(userPixmap != QPixmap()){
        QPixmap scaled = userPixmap.scaled(ui->userImage->width(), ui->userImage->height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        ui->userImage->setPixmap(scaled);
    }

    else {
        QString styleSheet = "QLabel { background-color: rgb(255, 254, 239); color: black; border-style: solid; border-width: 1.2px; border-color: black;}";
        ui->userImage->setStyleSheet(styleSheet);
        QFont font("Arial", 50);
        ui->userImage->setFont(font);
        ui->userImage->setText("U"); //Si dovrebbe impostare la propria iniziale
    }

    this->show();
}

Account::~Account()
{
    delete ui;
}

void Account::on_pushButton_clicked()
{
    // salvare i cambiamenti nel db -- TO DO
    QString name = ui->lineEdit->text();
    QString surname = ui->lineEdit_2->text();
    QString nick = ui->lineEdit_3->text();
    QString mail = ui->lineEdit_4->text();
    this->close();
}



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
    pal.setColor(QPalette::Background, QColor(128,128,128));
    pal.setColor(QPalette::WindowText, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->show();

    // set picture
    QPixmap pix("path -- TO DO");
    ui->label->setPixmap(pix);

    connect( this, SIGNAL(openThisFile(QString)),
             this->socket, SLOT(sendOpenFile(QString)));
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
}

void Account::on_pushButton_2_clicked()
{
    //tornare alla schermata precedente
    MainWindow *main;
    main = new MainWindow(this->socket, this->socket->getFHandler(), this, name);
    hide();
    main->show();
    emit openThisFile(name);
    hide();
}

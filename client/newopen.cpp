#include "newopen.h"
#include "ui_newopen.h"
#include <QShortcut>

NewOpen::NewOpen(Socket *sock, FileHandler *fHandler, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewOpen),
    socket(sock),
    fileHandler(fHandler)
{
    ui->setupUi(this);
    QPalette pal = palette();

    // set black background
    pal.setColor(QPalette::Background, QColor(58,58,60));
    pal.setColor(QPalette::WindowText, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    QPalette p = ui->listWidget->palette();
    p.setColor(QPalette::Base, QColor(209,209,214));
    p.setColor(QPalette::Text, Qt::black);
    p.setColor(QPalette::Button, QColor(229,229,234));
    ui->listWidget->setPalette(p);
    ui->lineEdit->setPalette(p);
    ui->lineEdit_2->setPalette(p);
    ui->pushButton->setPalette(p);
    ui->pushButton_2->setPalette(p);
    ui->pushButton_3->setPalette(p);
    this->show();
    setWindowTitle("");

    for (QString s : this->socket->getMapFiles().keys()){
        ui->listWidget->addItem(s);
    }

    connect( this, SIGNAL(openThisFile(QString)),
             this->socket, SLOT(sendOpenFile(QString)));

    connect( this, SIGNAL(newFile(QString)),
             this->socket, SLOT(sendNewFile(QString)));
    connect(this, SIGNAL(checkUri(QString)),
            this->socket, SLOT(sendAccess(QString)));
    connect(socket, SIGNAL(uriIsOk(QString)),
             this, SLOT(uriIsOk(QString)));
    connect(socket, SIGNAL(uriIsNotOk()),
             this, SLOT(uriIsNotOk()));

    QShortcut *sc = new QShortcut(QKeySequence("Return"),this);
    connect(sc, SIGNAL(activated()), ui->pushButton, SLOT(click()));
}

NewOpen::~NewOpen()
{
    delete ui;
}

void NewOpen::on_pushButton_2_clicked() //Bottone: new Document
{
    QString newfile = ui->lineEdit_2->text();
    emit newFile(newfile);
    mainwindow = new MainWindow(this->socket, this->socket->getFHandler(), this, newfile);
    hide();
    mainwindow->show();
}

void NewOpen::on_pushButton_clicked() //Bottone: open Document
{
    QString n = ui->listWidget->currentItem()->text();
    mainwindow = new MainWindow(this->socket, this->socket->getFHandler(), this, n);
    hide();
    mainwindow->show();
    emit openThisFile(n);
    hide();
}

void NewOpen::on_pushButton_3_clicked() //Bottone: uri
{
    QString uri = ui->lineEdit->text();
    emit checkUri(uri);
}

void NewOpen::uriIsOk(QString uri){
    ui->listWidget->addItem(uri);
    ui->listWidget->findItems(uri,Qt::MatchExactly).first()->setBackground(QColor(52,199,89));

}

void NewOpen::uriIsNotOk(){
    uri = new Uri(socket,this);
    uri->show();
}

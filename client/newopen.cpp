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
    pal.setColor(QPalette::Background, QColor(128,128,128));
    pal.setColor(QPalette::WindowText, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->show();

    for (QString s : this->socket->getMapFiles().keys()){
        ui->listWidget->addItem(s);
    }

    connect( this, SIGNAL(openThisFile(QString)),
             this->socket, SLOT(sendOpenFile(QString)));

    connect( this, SIGNAL(newFile(QString)),
             this->socket, SLOT(sendNewFile(QString)));

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
    //emit checkUri(uri);
}

void NewOpen::uriIsOk(QString uri){

    ui->listWidget->addItem(uri);
    ui->listWidget->findItems(uri,Qt::MatchExactly).first()->setBackground(QColor(52,199,89));

}

void NewOpen::uriIsNotOk(){
    uri = new Uri(socket,this);
    uri->show();
}

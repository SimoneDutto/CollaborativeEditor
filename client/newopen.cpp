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
    mainwindow = new MainWindow(this->socket, this->socket->getFHandler(), this);
    hide();
    mainwindow->show();
}

void NewOpen::on_pushButton_clicked() //Bottone: open Document
{
    mainwindow = new MainWindow(this->socket, this->socket->getFHandler(), this);
    hide();
    mainwindow->show();
    emit openThisFile(ui->listWidget->currentItem()->text());
    hide();
}

void NewOpen::on_pushButton_3_clicked()
{
    QString uri = ui->lineEdit->text();
    //emit openUri(uri);
}


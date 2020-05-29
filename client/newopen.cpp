#include "newopen.h"
#include "ui_newopen.h"
#include <QShortcut>
#include <QProcess>
#include "error.h"

NewOpen::NewOpen(Socket *sock, FileHandler *fHandler, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewOpen),
    socket(sock),
    fileHandler(fHandler)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window);
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

    ui->lineEdit->setStyleSheet(":focus {border: 2px solid #ace4c6};");
    ui->lineEdit->setAttribute(Qt::WA_MacShowFocusRect,0);
    ui->lineEdit_2->setStyleSheet(":focus {border: 2px solid #ace4c6};");
    ui->lineEdit_2->setAttribute(Qt::WA_MacShowFocusRect,0);

    QIcon *user_icon= new QIcon(":/rec/icone/icons8-apri-cartella-96.png");
    ui->pushButton_5->setIcon(*user_icon);
    ui->pushButton_5->setIconSize(QSize(30, 30));

    user_icon= new QIcon(":/rec/icone/icons8-aggiungi-file-96.png");
    ui->pushButton_6->setIcon(*user_icon);
    ui->pushButton_6->setIconSize(QSize(30, 30));

    user_icon= new QIcon(":/rec/icone/icons8-aggiungi-collegamento-96.png");
    ui->pushButton_7->setIcon(*user_icon);
    ui->pushButton_7->setIconSize(QSize(30, 30));

    QString styleSheet = "QPushButton {background-color: transparent; border-style: none; color: white}";
    ui->pushButton_5->setStyleSheet(styleSheet);
    ui->pushButton_6->setStyleSheet(styleSheet);
    ui->pushButton_7->setStyleSheet(styleSheet);

    ui->pushButton->setStyleSheet("color: white; background-color: #706d82;  border-radius:5px");
    ui->pushButton_2->setStyleSheet("color: white; background-color: #706d82;  border-radius:5px");
    ui->pushButton_3->setStyleSheet("color: white; background-color: #706d82;  border-radius:5px");

    QString username = socket->getClientUsername();
    ui->username->setText(username);

    styleSheet = "QLabel { background-color: rgb(255, 252, 247); color: black; border-style: solid; border-width: 2px; border-radius: 6px; border-color: orange; font: ; }";
    ui->myicon->setStyleSheet(styleSheet);
    QFont font("Arial", 30);
    ui->myicon->setFont(font);

    QString imageName = QString::number(socket->getClientID())+".png";
    QPixmap userPixmap = QPixmap(imageName);

    QIcon *discard_icon= new QIcon(":/rec/icone/icons8-punta-della-matita-96.png");
    ui->discardImage->setIcon(*discard_icon);
    ui->discardImage->setIconSize(QSize(18, 18));

    styleSheet = "QPushButton {background-color: white; border-style: solid; border-width: 1px; border-radius: 15px; border-color: rgb(0, 0, 0);} QPushButton:hover {background-color: rgb(233, 233, 233)} QPushButton:pressed {background-color: rgb(181, 181, 181)}";
    ui->discardImage->setStyleSheet(styleSheet);

    if(userPixmap != QPixmap()){
        QPixmap scaled = userPixmap.scaled(ui->myicon->width(), ui->myicon->height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        ui->myicon->setPixmap(scaled);
    }

    else {
        ui->myicon->setText(username.at(0).toUpper());
    }


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
    connect(ui->label, SIGNAL(clicked()), this, SLOT(on_actionLog_Out_triggered()));

    QShortcut *sc = new QShortcut(QKeySequence("Return"),this);
    connect(sc, SIGNAL(activated()), ui->pushButton, SLOT(click()));
}

NewOpen::~NewOpen()
{
    delete account;
    delete ui;
}

void NewOpen::on_pushButton_2_clicked() //Bottone: new Document
{
    disconnect(socket, SIGNAL(uriIsOk(QString)),
             this, SLOT(uriIsOk(QString)));
    disconnect(socket, SIGNAL(uriIsNotOk()),
             this, SLOT(uriIsNotOk()));
    QString newfile = ui->lineEdit_2->text();
    if(newfile.isEmpty()){
        Error *e = new Error(this);
        e->show();
    }
    else{
        emit newFile(newfile);
        mainwindow = new MainWindow(this->socket, this->socket->getFHandler(), this, newfile);
        hide();
        mainwindow->show();
    }

}

void NewOpen::on_pushButton_clicked() //Bottone: open Document
{
    disconnect(socket, SIGNAL(uriIsOk(QString)),
             this, SLOT(uriIsOk(QString)));
    disconnect(socket, SIGNAL(uriIsNotOk()),
             this, SLOT(uriIsNotOk()));
    QString n = ui->listWidget->currentItem()->text();
    emit openThisFile(n);
    mainwindow = new MainWindow(this->socket, this->socket->getFHandler(), this, n);
    hide();
    mainwindow->show();
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

void NewOpen::changeClientImage(QString imageName){

    if(imageName != "") {
        QPixmap userPixmap = QPixmap(imageName);

        if(userPixmap != QPixmap()){
            QPixmap scaled = userPixmap.scaled(ui->myicon->width(), ui->myicon->height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            ui->myicon->setPixmap(scaled);
        }

        else ui->myicon->setText(socket->getClientUsername().at(0).toUpper());
    }

    else ui->myicon->setText(socket->getClientUsername().at(0).toUpper());

}

void NewOpen::on_discardImage_clicked()
{
    account = new Account(this->socket, this, this->windowTitle());
    account->show();
}

void NewOpen::on_actionLog_Out_triggered()
{
    emit logOut();
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}


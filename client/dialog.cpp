#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(Socket *sock, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
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
    QPalette p = ui->listWidget->palette(); // define pallete for textEdit..
    p.setColor(QPalette::Base, QColor(209,209,214)); // set color "Red" for textedit base
    p.setColor(QPalette::Text, Qt::black); // set text color which is selected from color pallete
    ui->listWidget->setPalette(p);
    ui->lineEdit->setPalette(p);
    this->show();
    setWindowTitle("Files");
    ui->lineEdit->setStyleSheet(":focus {border: 2px solid #ace4c6};");
    ui->lineEdit->setAttribute(Qt::WA_MacShowFocusRect,0);

    QIcon *user_icon= new QIcon(":/rec/icone/icons8-aggiungi-collegamento-96.png");
    ui->pushButton_3->setIcon(*user_icon);
    ui->pushButton_3->setIconSize(QSize(30, 30));
    user_icon= new QIcon(":/rec/icone/icons8-aggiungi-file-96.png");
    ui->pushButton_4->setIcon(*user_icon);
    ui->pushButton_4->setIconSize(QSize(30, 30));

    QString styleSheet = "QPushButton {background-color: transparent; border-style: none; color: white}";
    ui->pushButton_3->setStyleSheet(styleSheet);
    ui->pushButton_4->setStyleSheet(styleSheet);

    ui->pushButton->setStyleSheet("color: white; background-color: #706d82;  border-radius:5px");
    ui->pushButton_2->setStyleSheet("color: white; background-color: #706d82;  border-radius:5px");
    /*
    listFiles.append("Prova1");
    listFiles.append("Prova2");
    listFiles.append("Prova3");
    listFiles.append("Prova4");
    listFiles.append("Prova5");
    */

    for (QString s : this->socket->getMapFiles().keys()){
        ui->listWidget->addItem(s);
    }

    connect( this, SIGNAL(openThisFile(QString)),
             this->socket, SLOT(sendOpenFile(QString)));
    connect(this, SIGNAL(checkUri(QString)),
            this->socket, SLOT(sendAccess(QString)));
    connect(socket, SIGNAL(uriIsOk(QString)),
             this, SLOT(uriIsOk(QString)));
    connect(socket, SIGNAL(uriIsNotOk()),
             this, SLOT(uriIsNotOk()));
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButton_clicked()
{
    emit openThisFile(ui->listWidget->currentItem()->text());
    hide();
}

void Dialog::on_pushButton_2_clicked()
{
    QString uri = ui->lineEdit->text();
    emit checkUri(uri);
}

void Dialog::uriIsOk(QString uri){
    ui->listWidget->addItem(uri);
    ui->listWidget->findItems(uri,Qt::MatchExactly).first()->setBackground(QColor(52,199,89));

}

void Dialog::uriIsNotOk(){
    Uri *uri = new Uri(socket,this);
    uri->show();
}

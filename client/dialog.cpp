#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(Socket *sock, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    socket(sock)
{
    ui->setupUi(this);
    setWindowTitle("Files");
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

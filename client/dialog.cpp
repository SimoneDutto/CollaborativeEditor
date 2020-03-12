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
    this->show();
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

#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    setWindowTitle("Files");
    listFiles.append("Prova1");
    listFiles.append("Prova2");
    listFiles.append("Prova3");
    listFiles.append("Prova4");
    listFiles.append("Prova5");
    for (QString s : this->listFiles){
        ui->listWidget->addItem(s);
    }
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButton_clicked()
{
    //emit openFile(ui->listWidget->currentItem()->text());
    hide();
}

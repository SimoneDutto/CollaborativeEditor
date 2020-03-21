#include "serverdisc.h"
#include "ui_serverdisc.h"
#include <QProcess>

serverDisc::serverDisc(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::serverDisc)
{
    ui->setupUi(this);
    setWindowTitle("Server Error");
    setWindowModality(Qt::WindowModal);
}

serverDisc::~serverDisc()
{
    delete ui;
}

void serverDisc::on_pushButton_clicked()
{
    emit logOut();
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}

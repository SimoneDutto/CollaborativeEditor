#include "uri.h"
#include "ui_uri.h"

Uri::Uri(Socket *s, QWidget *parent, QString uri) :
    QDialog(parent),
    ui(new Ui::Uri),
    sock(s)
{
    ui->setupUi(this);
    ui->label_2->setText(uri);
}

Uri::~Uri()
{
    delete ui;
}

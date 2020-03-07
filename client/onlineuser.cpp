#include "onlineuser.h"
#include "ui_onlineuser.h"

OnlineUser::OnlineUser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OnlineUser)
{
    ui->setupUi(this);
    this->show();

    QListWidgetItem *item = new QListWidgetItem("user1");
    item->setTextColor(Qt::red);
    //item->setSizeHint(QSize(40,40)); è lo spazio tra una riga e l'altra

    QFont font( "lucida", 30, QFont::Bold, true);

    item->setFont(font);

    ui->listOnlineUser->addItem(item);

    QListWidgetItem *fra = new QListWidgetItem("user2");
    fra->setTextColor(Qt::blue);
    ui->listOnlineUser->addItem(fra);

    //QIcon icon;


    //QListWidgetItem *item = new QListWidgetItem(icon, "ciao");
}

OnlineUser::~OnlineUser()
{
    delete ui;
}



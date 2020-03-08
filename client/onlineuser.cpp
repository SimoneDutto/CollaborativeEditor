#include "onlineuser.h"
#include "ui_onlineuser.h"

OnlineUser::OnlineUser(Socket *sock, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OnlineUser),
    socket(sock)
{
    ui->setupUi(this);
    setWindowTitle("Online Users");
    QMap<int, QColor> usersOnline = socket->getUserColor();

    /* User a caso per testing */
    usersOnline.insert(1, QColor("black"));
    usersOnline.insert(2, QColor("red"));
    usersOnline.insert(3, QColor("green"));
    usersOnline.insert(4, QColor("yellow"));
    usersOnline.insert(5, QColor("purple"));
    usersOnline.insert(6, QColor("purple"));
    usersOnline.insert(7, QColor("purple"));
    usersOnline.insert(8, QColor("purple"));
    usersOnline.insert(9, QColor("purple"));
    usersOnline.insert(10, QColor("purple"));
    usersOnline.insert(11, QColor("purple"));

    QList<int> keys = usersOnline.keys();

    for(int siteID : keys){
        QListWidgetItem *user = new QListWidgetItem(QString::number(siteID));
        user->setForeground(usersOnline.take(siteID));
        //item->setSizeHint(QSize(40,40)); è lo spazio tra una riga e l'altra
        QFont font( "", 20, QFont::Bold);
        user->setFont(font);
        user->setFlags(Qt::ItemIsEnabled);

        ui->listOnlineUser->addItem(user);
    }

    this->show();
}

OnlineUser::~OnlineUser()
{
    delete ui;
}



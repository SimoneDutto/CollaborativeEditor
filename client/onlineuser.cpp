#include "onlineuser.h"
#include "ui_onlineuser.h"

OnlineUser::OnlineUser(Socket *sock, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OnlineUser),
    socket(sock)
{
    ui->setupUi(this);
    setWindowTitle("Online Users");
    QMap<QString, QColor> usersOnline = socket->getUserColor();

    /* User a caso per testing
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
    usersOnline.insert(11, QColor("purple")); */

    QList<QString> keys = usersOnline.keys();

    for(QString username : keys){
        QListWidgetItem *user = new QListWidgetItem(username);
        user->setForeground(usersOnline.take(username));
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



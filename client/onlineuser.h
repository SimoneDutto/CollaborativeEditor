#ifndef ONLINEUSER_H
#define ONLINEUSER_H

#include <QDialog>
#include <QListWidgetItem>
#include <QListWidget>
#include <QPalette>
#include "socket.h"

namespace Ui {
class OnlineUser;
}

class OnlineUser : public QDialog
{
    Q_OBJECT

public:
    explicit OnlineUser(Socket *sock = nullptr, QWidget *parent = nullptr);
    ~OnlineUser();

private:
    Ui::OnlineUser *ui;
    Socket *socket;
};

#endif // ONLINEUSER_H

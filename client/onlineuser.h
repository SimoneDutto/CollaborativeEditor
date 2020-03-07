#ifndef ONLINEUSER_H
#define ONLINEUSER_H

#include <QDialog>
#include <QListWidgetItem>
#include <QListWidget>

namespace Ui {
class OnlineUser;
}

class OnlineUser : public QDialog
{
    Q_OBJECT

public:
    explicit OnlineUser(QWidget *parent = nullptr);
    ~OnlineUser();

private:
    Ui::OnlineUser *ui;
};

#endif // ONLINEUSER_H

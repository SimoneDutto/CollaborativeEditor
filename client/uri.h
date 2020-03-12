#ifndef URI_H
#define URI_H

#include <QDialog>
#include "socket.h"

namespace Ui {
class Uri;
}

class Uri : public QDialog
{
    Q_OBJECT

public:
    explicit Uri(Socket *sock, QWidget *parent = nullptr);
    ~Uri();

private:
    Ui::Uri *ui;
    Socket *sock;
};

#endif // URI_H

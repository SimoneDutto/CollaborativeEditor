#include "login.h"
#include "socket.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Socket sock;
    //sock.Connect();

    Login w;
    w.show();
    return a.exec();
}

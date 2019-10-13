#include "login.h"
#include "socket.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Socket sock("localhost", 4001);
    //sock.openFile("text.txt");

    //Login w(&sock);
    //w.show();

    MainWindow m(&sock);
    m.show();
    return a.exec();
}

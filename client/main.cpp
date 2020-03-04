#include "welcome.h"
#include "socket.h"
#include <QtQuickControls2>
#include <QQuickStyle>
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Socket sock("localhost", 4001);
    Welcome w(&sock);
    w.show();

    //MainWindow m(&sock);
    //m.show();
    return a.exec();
}

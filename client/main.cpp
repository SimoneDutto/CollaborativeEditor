#include "login.h"
#include "socket.h"
#include <QtQuickControls2>
#include <QQuickStyle>
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Socket sock("localhost", 4001);
    Login w(&sock);
    w.show();
    a.setWindowIcon(QIcon(":rec/icone/logo_transparent copy.png"));

    return a.exec();
}

#include "login.h"
#include "socket.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    /*Socket sock("localhost", 8080);
    sock.openFile("text.txt");*/

    QSettings settings;
    QString filePath = settings.value("Path/XmlTemporary", "").toString();
    qDebug() << "FilePath is: " << filePath;

    /*QApplication a(argc, argv);
    Socket sock;
    Login w;
    w.show();
    return a.exec();*/
}

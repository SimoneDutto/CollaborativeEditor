#include "login.h"
#include "socket.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    /*Socket sock("localhost", 8080);
    sock.openFile("text.txt");
    Login w;
    w.show();
    return a.exec();*/

    QApplication a(argc, argv);
    Socket sock("localhost", 4001);

    QVector<Letter> file;
    sock.openFile("text.txt");

    /*  PER TESTATE JSON FILE
    QFile inFile("/Users/vitotassielli/Desktop/document.json");
    inFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray data = inFile.readAll();
    inFile.close();*/
    return 0;
}

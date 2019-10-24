#include "myserver.h"
#include <QDir>

#include <QCoreApplication>
#include <QScopedPointer>

int main(int argc, char *argv[])
{
    QScopedPointer<QCoreApplication> app(new QCoreApplication(argc, argv));

    QScopedPointer<MyServer> server(new MyServer());

    if (!server->listen(QHostAddress::LocalHost, 4001))
        return 1;

    return app->exec();
}

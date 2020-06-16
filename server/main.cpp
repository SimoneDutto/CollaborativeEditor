#include "myserver.h"

#include <QCoreApplication>
#include <QScopedPointer>

int main(int argc, char *argv[])
{
    QScopedPointer<QCoreApplication> app(new QCoreApplication(argc, argv));
    QScopedPointer<MyServer> server(new MyServer());

    if (!server->listen(QHostAddress::Any, 4001))
        return 1;
    return app->exec();
}

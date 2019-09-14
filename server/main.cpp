#include "server.h"

#include <QCoreApplication>
#include <QScopedPointer>

int main(int argc, char *argv[])
{
    QScopedPointer<QCoreApplication> app(new QCoreApplication(argc, argv));

    QScopedPointer<Server> server(new Server());

    if (!server->listen(QHostAddress(QStringLiteral("0.0.0.0")), 4000))
        return 1;

    return app->exec();
}

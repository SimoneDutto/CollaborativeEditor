#include "server.h"

#include <QSignalMapper>
#include <QTcpServer>
#include <QTcpSocket>

Server::Server(QObject *parent) :
    QObject(parent),
    m_server(new QTcpServer(this)),
    m_readyReadSignalMapper(new QSignalMapper(this)),
    m_disconnectedSignalMapper(new QSignalMapper(this))
{
    connect(m_server, SIGNAL(newConnection()), SLOT(onNewConnection()));
    connect(m_readyReadSignalMapper, SIGNAL(mapped(QObject*)), SLOT(onReadyRead(QObject*)));
    connect(m_disconnectedSignalMapper, SIGNAL(mapped(QObject*)), SLOT(onDisconnected(QObject*)));
}

bool Server::listen(const QHostAddress &address, quint16 port)
{
    if (!m_server->listen(address, port)) {
        qCritical("Cannot start server: %s", qPrintable(m_server->errorString()));
        return false;
    }

    qDebug("Echo server is listening on %s:%d.", qPrintable(address.toString()), port);

    return true;
}

void Server::onNewConnection()
{
    QTcpSocket *socket = m_server->nextPendingConnection();
    if (!socket)
        return;

    qDebug("New connection from %s:%d.",
           qPrintable(socket->peerAddress().toString()), socket->peerPort());

    connect(socket, SIGNAL(readyRead()), m_readyReadSignalMapper, SLOT(map()));
    m_readyReadSignalMapper->setMapping(socket, socket);

    connect(socket, SIGNAL(disconnected()), m_disconnectedSignalMapper, SLOT(map()));
    m_disconnectedSignalMapper->setMapping(socket, socket);
}

void Server::onReadyRead(QObject *socketObject)
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(socketObject);
    if (!socket || !socket->bytesAvailable())
        return;

    QByteArray ba = socket->readAll();
    if (ba.isEmpty())
        return;

    qDebug("Received data from %s:%d.",
           qPrintable(socket->peerAddress().toString()), socket->peerPort());
    socket->write(ba);
}

void Server::onDisconnected(QObject *socketObject)
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(socketObject);
    if (!socket)
        return;

    qDebug("Client %s:%d has disconnected.",
           qPrintable(socket->peerAddress().toString()), socket->peerPort());

    socket->deleteLater();
}

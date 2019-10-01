#include "myserver.h"
#include "filesystem.h"
#include "letter.h"
#include "filehandler.h"

#include <QSignalMapper>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

MyServer::MyServer(QObject *parent) :
    QObject(parent),
    m_server(new QTcpServer(this)),
    m_readyReadSignalMapper(new QSignalMapper(this)),
    m_disconnectedSignalMapper(new QSignalMapper(this))
{
    fsys = FileSystem::getInstance();
    connect(m_server, SIGNAL(newConnection()), SLOT(onNewConnection()));
    connect(m_readyReadSignalMapper, SIGNAL(mapped(QObject*)), SLOT(onReadyRead(QObject*)));
    connect(m_disconnectedSignalMapper, SIGNAL(mapped(QObject*)), SLOT(onDisconnected(QObject*)));
}

bool MyServer::listen(const QHostAddress &address, quint16 port)
{
    if (!m_server->listen(address, port)) {
        qCritical("Cannot start server: %s", qPrintable(m_server->errorString()));
        return false;
    }

    qDebug("Echo server is listening on %s:%d.", qPrintable(address.toString()), port);

    return true;
}

void MyServer::onNewConnection()
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

void MyServer::onReadyRead(QObject *socketObject)
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(socketObject);
    if (!socket || !socket->bytesAvailable())
        return;
    QByteArray str = socket->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(str);
    QJsonObject rootObject = jsonResponse.object();

    QString type = rootObject.value(("type")).toString();
    qDebug() << "Tipo di richiesta: " << type;
    if(type.compare("OPEN")==0){
        qDebug() << "OPEN request";
        QString filename = rootObject.value(("filename")).toString();
        fsys->sendFile(filename, socket);
    }
    else if(type.compare("insert")){
        qDebug() << "INSERT request";
        QString filename = rootObject.value(("filename")).toString();
        if(fsys->getFiles().find(filename) != fsys->getFiles().end()) {     // file exists
            FileHandler* fHandler = fsys->getFiles().at(filename);
            QChar newLetterValue = rootObject.value("letter").toString().at(0);
            QJsonArray position = rootObject.value("position").toArray();
            int externalIndex = rootObject.value("externalIndex").toInt();
            int siteID = rootObject.value("siteID").toInt();
            int siteCounter = rootObject.value("siteCounter").toInt();
            fHandler->remoteInsert(position, newLetterValue, externalIndex, siteID, siteCounter);
        }

    }
    else if(type.compare("delete")){
        qDebug() << "DELETE request";
        QString filename = rootObject.value("filename").toString();
        if(fsys->getFiles().find(filename) != fsys->getFiles().end()) {     // file exists
            FileHandler* fHandler = fsys->getFiles().at(filename);
            QString deletedLetterID = rootObject.value("letterID").toString();
            fHandler->remoteDelete(deletedLetterID);
        }
    }
}

void MyServer::onDisconnected(QObject *socketObject)
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(socketObject);
    if (!socket)
        return;

    qDebug("Client %s:%d has disconnected.",
           qPrintable(socket->peerAddress().toString()), socket->peerPort());

    socket->deleteLater();
}

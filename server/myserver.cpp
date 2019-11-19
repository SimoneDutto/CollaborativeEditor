#include "myserver.h"
#include "filesystem.h"
#include "letter.h"
#include "filehandler.h"

#include <QSignalMapper>
#include <QTcpServer>

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
    connect(fsys, SIGNAL(signUpResponse(QString,bool,QTcpSocket*)), this, SLOT(sendSignUpResponse(QString,bool,QTcpSocket*)));
    connect(fsys, SIGNAL(dataRead(QByteArray, QTcpSocket*, int)),this, SLOT(sendFileChunk(QByteArray, QTcpSocket*, int)));
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
    qDebug() << "Tipo di richiesta: " << str.data();
    if(type.compare("OPEN")==0){
        qDebug() << "OPEN request";
        int fileid = rootObject.value(("fileid")).toInt();

        FileHandler *fh = fsys->sendFile(fileid, socket);
        if(fh == nullptr) return;
        /* Connect socket to signals for remote insert and delete */

        connect(fh, SIGNAL(remoteInsertNotify(QVector<QTcpSocket*>, QByteArray, bool, int, QTcpSocket*)),
                this, SLOT(sendInsert(QVector<QTcpSocket*>, QByteArray, bool, int, QTcpSocket*)));

        connect(fh, SIGNAL(remoteDeleteNotify(QVector<QTcpSocket*>, QByteArray)),
                this, SLOT(sendDelete(QVector<QTcpSocket*>, QByteArray)));
    }
    else if(type.compare("NEW")==0){
        qDebug() << "NEW request";
        QString filename = rootObject.value(("filename")).toString();

        FileHandler *fh = fsys->createFile(filename, socket);
        if(fh == nullptr) return;
        connect(fh, SIGNAL(remoteInsertNotify(QVector<QTcpSocket*>, QByteArray, bool, int)),
                this, SLOT(sendInsert(QVector<QTcpSocket*>, QByteArray, bool, int)));

        connect(fh, SIGNAL(remoteDeleteNotify(QVector<QTcpSocket*>, QByteArray)),
                this, SLOT(sendDelete(QVector<QTcpSocket*>, QByteArray)));
    }
    else if(type.compare("INSERT")==0){
        qDebug() << "INSERT request";
        int fileid = rootObject.value(("fileid")).toInt();
        if(fsys->getFiles().find(fileid) != fsys->getFiles().end()) {     // file exists
            FileHandler* fHandler = fsys->getFiles().at(fileid);
            QChar newLetterValue = rootObject.value("letter").toString().at(0);
            QJsonArray position = rootObject.value("position").toArray();
            int externalIndex = rootObject.value("externalIndex").toInt();
            int siteID = rootObject.value("siteID").toInt();
            int siteCounter = rootObject.value("siteCounter").toInt();
            fHandler->remoteInsert(position, newLetterValue, externalIndex, siteID, siteCounter, str, socket);
        }
    }
    else if(type.compare("DELETE")==0){
        qDebug() << "DELETE request";
        int fileid = rootObject.value(("fileid")).toInt();
        if(fsys->getFiles().find(fileid) != fsys->getFiles().end()) {     // file exists
            FileHandler* fHandler = fsys->getFiles().at(fileid);
            QString deletedLetterID = rootObject.value("letterID").toString();
            fHandler->remoteDelete(deletedLetterID, str);
        }
    }
    else if(type.compare("LOGIN")==0){
        QString username = rootObject.value(("nickname")).toString();
        QString password = rootObject.value(("password")).toString();
        fsys->checkLogin(username, password, socket);
    }
    else if(type.compare("SIGNUP")==0) {
        QString username = rootObject.value("username").toString();
        QString psw = rootObject.value("password").toString();
        fsys->storeNewUser(username, psw, socket);
    }
}

void MyServer::onDisconnected(QObject *socketObject)
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(socketObject);
    if (!socket)
        return;
    fsys->disconnectClient(socket);
    qDebug("Client %s:%d has disconnected.",
           qPrintable(socket->peerAddress().toString()), socket->peerPort());

    socket->deleteLater();
}

void MyServer::sendInsert(QVector<QTcpSocket*> users, QByteArray message, bool modifiedIndex, int newIndex, QTcpSocket *client) {
    QJsonObject obj;
    if(modifiedIndex) {
        // Edit json file
        QJsonDocument jsonResponse = QJsonDocument::fromJson(message);
        QJsonObject rootObject = jsonResponse.object();
        obj.insert("type", "INSERT");
        obj.insert("filename", rootObject.value("filename").toString());
        obj.insert("letter", rootObject.value("letter").toString());
        obj.insert("position", rootObject.value("position").toArray());
        obj.insert("siteID", rootObject.value("siteID").toString());
        obj.insert("siteCounter", rootObject.value("siteCounter").toInt());
        obj.insert("externalIndex", newIndex);
    }

    QVectorIterator<QTcpSocket*> i(users);
    while (i.hasNext()){
        QTcpSocket* socket = i.next();
        if(socket == client) continue;
        if(socket->state() == QAbstractSocket::ConnectedState) {
            if(modifiedIndex) {
                socket->write(QJsonDocument(obj).toJson()); //write size of data
            } else
                socket->write(message);
            socket->waitForBytesWritten(1000);
        }
    }
}

void MyServer::sendDelete(QVector<QTcpSocket*> users, QByteArray message){
    QVectorIterator<QTcpSocket*> i(users);
    while (i.hasNext()){
        QTcpSocket* socket = i.next();
        if(socket->state() == QAbstractSocket::ConnectedState) {
            socket->write(message);
            socket->waitForBytesWritten(1000);
        }
    }
}

void MyServer::sendSignUpResponse(QString message, bool success, QTcpSocket* socket) {
    QJsonObject json;
    json.insert("type", "SIGNUP_RESPONSE");
    json.insert("success", success);
    json.insert("msg", message);
    if(socket->state() == QAbstractSocket::ConnectedState) {
        socket->write(QJsonDocument(json).toJson());
        socket->waitForBytesWritten(1000);
    }
}

void MyServer::sendFileChunk(QByteArray chunk, QTcpSocket* socket, int remainingSize) {
    QJsonObject object;
    QJsonArray array;
    qDebug() <<"Sono qui";

    QString s_data = chunk.data();
    object.insert("type", "FILE");
    object.insert("chunk", s_data);
    object.insert("remaining", remainingSize);
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        qDebug() << "Invio file";
        socket->write(FileSystem::IntToArray(QJsonDocument(object).toJson().size()));
        socket->write(QJsonDocument(object).toJson());
        socket->waitForBytesWritten();
    }
}

#include "myserver.h"
#include "filesystem.h"
#include "letter.h"
#include "filehandler.h"

#include <QSignalMapper>
#include <QTcpServer>
#include <QDataStream>

MyServer::MyServer(QObject *parent) :
    QObject(parent),
    m_server(new QTcpServer(this))
{
    fsys = FileSystem::getInstance();
    connect(m_server, SIGNAL(newConnection()), SLOT(onNewConnection()));
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

    connect(socket, SIGNAL(readyRead()),this, SLOT(readBuffer()));
    //connect(socket, SIGNAL(disconnected()),this,  SLOT(onDisconnected(QTcpSocket*)));
    connect(this, SIGNAL(bufferReady(QTcpSocket*, QByteArray)), SLOT(handleNotifications(QTcpSocket*,QByteArray)));
}

void MyServer::readBuffer(){
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    QByteArray data;
    Buffer* buffer;

    auto buffer_f = barray_psocket.find(socket);

    if(buffer_f != barray_psocket.end()){
        buffer = buffer_f.value();
    }
    else{
        buffer = new Buffer();
        barray_psocket.insert(socket, buffer);
    }

    while (socket->bytesAvailable() > 0 || buffer->data.size() != 0 )
    {
       qDebug() << "Leggo dal socket";
       buffer->data.append(socket->readAll());
       while ((buffer->dim == 0 && buffer->data.size() >= 8) || (buffer->dim > 0 && buffer->data.size() >= buffer->dim)) //While can process data, process it
       {
           if (buffer->dim == 0 && buffer->data.size() >= 8) //if size of data has received completely, then store it on our global variable
           {
               buffer->dim = atoi(buffer->data.mid(0, 8).data());
               qDebug() << "Size: " << buffer->dim;
               buffer->data.remove(0, 8);
           }
           if (buffer->dim > 0 && buffer->data.size() >= buffer->dim) // If data has received completely, then emit our SIGNAL with the data
           {
               data = buffer->data.mid(0, (int)buffer->dim);
               buffer->data.remove(0,(int)buffer->dim);
               buffer->dim = 0;
               qDebug() << "Data: " << data.data();
               emit bufferReady(socket, data);
           }
       }
    }
}

void MyServer::handleNotifications(QTcpSocket *socket, QByteArray data)
{

    QJsonDocument jsonResponse = QJsonDocument::fromJson(data);
    QJsonObject rootObject = jsonResponse.object();

    QString type = rootObject.value(("type")).toString();
    qDebug() << "Richiesta: " << data.data();

    if(type.compare("OPEN")==0){
        qDebug() << "OPEN request";
        int fileid = rootObject.value(("fileid")).toInt();

        FileHandler *fh = fsys->sendFile(fileid, socket);
        if(fh == nullptr) return;
        /* Connect socket to signals for remote insert and delete */
    }
    else if(type.compare("NEW")==0){
        qDebug() << "NEW request";
        QString filename = rootObject.value(("filename")).toString();

        FileHandler *fh = fsys->createFile(filename, socket);
        if(fh == nullptr) return;
        //connect(fh, SIGNAL(remoteInsertNotify(QVector<QTcpSocket*>, QByteArray, bool, int)),
                //this, SLOT(sendInsert(QVector<QTcpSocket*>, QByteArray, bool, int)));

        //connect(fh, SIGNAL(remoteDeleteNotify(QVector<QTcpSocket*>, QByteArray)),
                //this, SLOT(sendDelete(QVector<QTcpSocket*>, QByteArray)));
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
            fHandler->remoteInsert(position, newLetterValue, externalIndex, siteID, siteCounter, data, socket);
        }
    }
    else if(type.compare("DELETE")==0){
        qDebug() << "DELETE request";
        int fileid = rootObject.value(("fileid")).toInt();
        if(fsys->getFiles().find(fileid) != fsys->getFiles().end()) {     // file exists
            FileHandler* fHandler = fsys->getFiles().at(fileid);
            QString deletedLetterID = rootObject.value("letterID").toString();
            fHandler->remoteDelete(deletedLetterID, data);
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

void MyServer::onDisconnected(QTcpSocket *socket)
{
    if (!socket)
        return;
    fsys->disconnectClient(socket);
    qDebug("Client %s:%d has disconnected.",
           qPrintable(socket->peerAddress().toString()), socket->peerPort());

    socket->deleteLater();
}


void MyServer::sendSignUpResponse(QString message, bool success, QTcpSocket* socket) {
    QJsonObject json;
    QByteArray sendSize;

    json.insert("type", "SIGNUP_RESPONSE");
    json.insert("success", success);
    json.insert("msg", message);

    if(socket->state() == QAbstractSocket::ConnectedState) {
        socket->write(sendSize.number(message.size()), sizeof (long int));
        socket->waitForBytesWritten();
        socket->write(QJsonDocument(json).toJson());
        socket->waitForBytesWritten(1000);
    }
}

void MyServer::sendFileChunk(QByteArray chunk, QTcpSocket* socket, int remainingSize) {
    QJsonObject object;
    QByteArray toSend;

    QString s_data = chunk.data();
    object.insert("type", "FILE");
    object.insert("chunk", s_data);
    object.insert("remaining", remainingSize);
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        //qDebug() << "Invio file";
        qDebug() << "size: " << QJsonDocument(object).toJson().size();
        qDebug() << "file with content: " << object;
        socket->write(toSend.number(QJsonDocument(object).toJson().size()), sizeof (long int));
        socket->waitForBytesWritten();
        socket->write(QJsonDocument(object).toJson());
        socket->waitForBytesWritten();
    }
}

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
    connect(fsys, SIGNAL(dataRead(QByteArray, QTcpSocket*, int, QString)),this, SLOT(sendFileChunk(QByteArray, QTcpSocket*, int, QString)));
    connect(this, SIGNAL(bufferReady(QTcpSocket*, QByteArray)), SLOT(handleNotifications(QTcpSocket*,QByteArray)));
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
    connect(socket, SIGNAL(disconnected()),this,  SLOT(onDisconnected()));

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
               data = buffer->data.mid(0, static_cast<int>(buffer->dim));
               buffer->data.remove(0, static_cast<int>(buffer->dim));
               buffer->dim = 0;
               //qDebug() << "Data: " << data.data();
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

    /*
     * OPEN
     * ACCESS
     * NEW
     * INSERT
     * DELETE
     * LOGIN
     * SIGNUP
     * STYLE
    */

    if(type.compare("OPEN")==0){
        qDebug() << "OPEN request";
        int fileid = rootObject.value(("fileid")).toInt();

        fsys->sendFile(fileid, socket);
        /* Connect socket to signals for remote insert and delete */
    }
    else if(type.compare("ACCESS")==0){
        qDebug() << "ACCESS request";
        QString URI = rootObject.value(("URI")).toString();

        fsys->accessFile(URI, socket);
    }
    else if(type.compare("NEW")==0){
        qDebug() << "NEW request";
        QString filename = rootObject.value(("filename")).toString();

        fsys->createFile(filename, socket);
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
            // Get letter format
            QTextCharFormat format;
            bool isBold = rootObject.value("isBold").toBool();
            bool isItalic = rootObject.value("isItalic").toBool();
            bool isUnderlined = rootObject.value("isUnderlined").toBool();

            if(isBold)
                format.setFontWeight(75);
            else format.setFontWeight(50);
            if(isItalic)
                format.setFontItalic(true);
            else format.setFontItalic(false);
            if(isUnderlined)
                format.setFontUnderline(true);
            else format.setFontUnderline(false);

            fHandler->remoteInsert(position, newLetterValue, externalIndex, siteID, siteCounter, data, socket, format);
        }
    }
    else if(type.compare("DELETE")==0){
        qDebug() << "DELETE request";
        int fileid = rootObject.value(("fileid")).toInt();
        if(fsys->getFiles().find(fileid) != fsys->getFiles().end()) {     // file exists
            FileHandler* fHandler = fsys->getFiles().at(fileid);
            QString deletedLetterID = rootObject.value("letterID").toString();
            int siteCounter = rootObject.value("siteCounter").toInt();
            fHandler->remoteDelete(deletedLetterID, data, socket, siteCounter);
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
    else if(type.compare("STYLE")==0) {
        int fileID = rootObject.value(("fileid")).toInt();
        if(fsys->getFiles().find(fileID) != fsys->getFiles().end()) {   // file exists
            FileHandler* fHandler = fsys->getFiles().at(fileID);
            QString initialIndex = rootObject.value("startIndex").toString();
            QString lastIndex = rootObject.value("lastIndex").toString();
            QString changedStyle = rootObject.value("changedStyle").toString();
            //int siteID = rootObject.value("siteID").toInt();
            //int siteCounter = rootObject.value("siteCounter").toInt();
            fHandler->changeStyle(initialIndex, lastIndex, changedStyle, socket, data);
        }
    }

}

void MyServer::onDisconnected()
{
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    if (!socket)
        return;
    fsys->disconnectClient(socket);
    qDebug("Client %s:%d has disconnected.",
           qPrintable(socket->peerAddress().toString()), socket->peerPort());

    socket->deleteLater();
}

void MyServer::sendFileChunk(QByteArray chunk, QTcpSocket* socket, int remainingSize, QString type) {
    QJsonObject object;
    QByteArray toSend;

    QString s_data = chunk.data();
    object.insert("type", type);
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

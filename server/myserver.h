
#ifndef ECHOSERVER_H
#define ECHOSERVER_H

#include <QHostAddress>
#include <QObject>
#include <QSignalMapper>
#include <QTcpSocket>
#include "buffer.h"
class QTcpServer;
class FileSystem;


class MyServer : public QObject
{
    Q_OBJECT
public:
    explicit MyServer(QObject *parent = nullptr);

    bool listen(const QHostAddress &address, quint16 port);

private slots:
    void onNewConnection();
    void handleNotifications(QTcpSocket *socket, QByteArray data);
    void onDisconnected();
    void sendFileChunk(QByteArray chunk, QTcpSocket *socket, int remainingSize, QString type);
    void readBuffer();
signals:
    void bufferReady(QTcpSocket* socket, QByteArray data);
private:
    FileSystem *fsys;
    QTcpServer *m_server;
    QMap<QTcpSocket*, Buffer*> barray_psocket;
};


#endif // ECHOSERVER_H

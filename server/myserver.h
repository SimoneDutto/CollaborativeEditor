
#ifndef ECHOSERVER_H
#define ECHOSERVER_H

#include <QHostAddress>
#include <QObject>
#include <QSignalMapper>
#include <QTcpSocket>

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
    void onReadyRead(QObject *socketObject);
    void onDisconnected(QObject *socketObject);

private:
    FileSystem *fsys;
    QTcpServer *m_server;
    QSignalMapper *m_readyReadSignalMapper;
    QSignalMapper *m_disconnectedSignalMapper;
};

#endif // ECHOSERVER_H

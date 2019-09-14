
#ifndef ECHOSERVER_H
#define ECHOSERVER_H

#include <QHostAddress>
#include <QObject>
#include <QSignalMapper>

class QTcpServer;

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);

    bool listen(const QHostAddress &address, quint16 port);

private slots:
    void onNewConnection();
    void onReadyRead(QObject *socketObject);
    void onDisconnected(QObject *socketObject);

private:
    QTcpServer *m_server;
    QSignalMapper *m_readyReadSignalMapper;
    QSignalMapper *m_disconnectedSignalMapper;
};

#endif // ECHOSERVER_H

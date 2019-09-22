#ifndef SOCKET_H
#define SOCKET_H

#include <QDialog>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonValueRef>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSettings>
#include <QFile>
#include "letter.h"
#include "iostream"

namespace Ui {
class Socket;
}

class Socket : public QDialog
{
    Q_OBJECT

public:
    explicit Socket(QWidget *parent = nullptr);
    Socket(const QString &host, quint16 port);
    ~Socket();

private:
    Ui::Socket *ui;
    QTcpSocket *socket;
    qint32 size;
    QByteArray buffer;

    int clientID;
    QVector<QString> listFiles;

public slots:
    int openFile(QString name_file);
    void checkLogin(QString username, QString password);
    int sendInsert(int pos, QString value);

private slots:
    void closeConnection();
    void socketReadyReadFile();
    void socketConnected();
    void socketConnectionClosed();
    void socketClosed();
    void socketError(int e);
    void socketReadyReadListFiles();

signals:
    void loginSuccess();
    void loginError();
};



#endif // SOCKET_H

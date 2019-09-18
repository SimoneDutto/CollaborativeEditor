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
    qint32 blockSize;
    QVector<Letter>* lastFilePtr;

public slots:
    int openFile(QString name_file);
    /*int openFile(QString name_file, QVector<Letter>& filePtr);
    openFile(QString name_file, QVector<Letter>& arrayFile)*/

private slots:
    void closeConnection();
    void sendToServer();
    void socketReadyReadFile();
    void socketConnected();
    void socketConnectionClosed();
    void socketClosed();
    void socketError(int e);
};



#endif // SOCKET_H

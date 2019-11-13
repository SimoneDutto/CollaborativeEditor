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
#include "filehandler.h"

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
    FileHandler* getFHandler();
    int getClientID();
    QVector<QString> getListFiles();

private:
    Ui::Socket *ui;
    QTcpSocket *socket;
    qint32 size;
    QByteArray buffer;
    FileHandler* fileh;
    int clientID;
    QVector<QString> listFiles;

public slots:
    void sendLogin(QString username, QString password);

private slots:
    void closeConnection();
    void socketConnected();
    void socketConnectionClosed();
    void socketClosed();
    void socketError(int e);

    void checkLoginAndGetListFileName();
    void notificationsHandler();

    int sendOpenFile(QString name_file);
    int sendInsert(QChar newLetterValue, QJsonArray position, int siteID, int siteCounter, int externalIndex);
    int sendDelete(int externalIndex);
    int sendCheckFileName(QString fileNameTmp);
    int sendNewFile(QString filename);

signals:
    void loginSuccess();
    void loginError();

    /*Notifiche durante l'uso*/
    void readyFile();
    void readyInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter);
    void readyDelete(QString deletedLetterID);
};



#endif // SOCKET_H

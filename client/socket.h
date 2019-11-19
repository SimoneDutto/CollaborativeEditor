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
#include <QMap>
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
    QMap<QString, int> getMapFiles();

private:
    Ui::Socket *ui;
    QTcpSocket *socket;
    QByteArray buffer;
    int fileid;
    FileHandler* fileh;
    int clientID;
    QMap<QString, int> mapFiles;

public slots:
    void sendSignUpRequest(QString username, QString password);
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
    /* Registrazione */
    void signUpSuccess();
    void signUpError();
    void invalidUsername();

    /* Login */
    void loginSuccess();
    void loginError();

    /*Notifiche durante l'uso*/
    void readyFile();
    void readyInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter);
    void readyDelete(QString deletedLetterID);
};



#endif // SOCKET_H

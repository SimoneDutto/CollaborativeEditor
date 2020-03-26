#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <map>
#include <string>
#include <QString>
#include <QTcpSocket>
#include <QDataStream>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QDir>
#include <QObject>
#include <QFile>

#include "letter.h"
#include "filehandler.h"


class FileHandler;
class QTcpSocket;

class FileSystem : public QObject
{
    Q_OBJECT
private:
    static FileSystem* instance;
    std::map<int, FileHandler*> files;
    std::map<QTcpSocket*, int> sock_id;
    std::map<QTcpSocket*, int> sock_file;
    std::map<QTcpSocket*, QString> sock_username;
    QSqlDatabase db;
    explicit FileSystem() {}

public:
    void sendFile(int fileid, QTcpSocket *socket);
    void createFile(QString filename, QTcpSocket *socket);
    void accessFile(QString URI, QTcpSocket *socket);
    void checkLogin(QString username, QString password, QTcpSocket *socket);
    void storeNewUser(QString username, QString psw, QTcpSocket *socket);
    static FileSystem* getInstance();
    void disconnectClient(QTcpSocket* socket);
    void updateFileSiteCounter(int fileID, int userID, int siteCounter);
    std::map<int, FileHandler*> getFiles();
    int getSocketID(QTcpSocket* socket);
    static void sendJson(QJsonObject json, QTcpSocket* socket);

signals:
    void signUpResponse(QString message, bool success, QTcpSocket* socket);
    void dataRead(QByteArray chunk, QTcpSocket* socket, int remainingSize);

private slots:
    void sendInsert(QVector<QTcpSocket*> users, QByteArray message, bool modifiedIndex, int newIndex, QTcpSocket* client);
    void sendDelete(QVector<QTcpSocket*> users, QByteArray message, QTcpSocket* client);
    void sendStyleChange(QVector<QTcpSocket*> users, QByteArray message, QTcpSocket* client);
    void sendCursorChange(QVector<QTcpSocket*> users, QByteArray message, QTcpSocket* client);
};

#endif // FILESYSTEM_H

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
    QSqlDatabase db;
    explicit FileSystem() {}

public:
    FileHandler* sendFile(int fileid, QTcpSocket *socket);
    FileHandler* createFile(QString filename, QTcpSocket *socket);
    void checkLogin(QString username, QString password, QTcpSocket *socket);
    void storeNewUser(QString username, QString psw, QTcpSocket *socket);
    static FileSystem* getInstance();
    std::map<int, FileHandler*> getFiles();
    void disconnectClient(QTcpSocket* socket);

signals:
    void signUpResponse(QString message, bool success, QTcpSocket* socket);
    void dataRead(QByteArray chunk, QTcpSocket* socket, int remainingSize);
};

#endif // FILESYSTEM_H

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

#include "letter.h"
#include "filehandler.h"

class FileHandler;
class QTcpSocket;

class FileSystem
{
    static FileSystem* instance;
    std::map<QString, FileHandler*> files;
    std::map<QTcpSocket*, int> sock_id;
    std::map<QTcpSocket*, QString> sock_file;
    QSqlDatabase db;
    FileSystem() {}
public:
    FileHandler* sendFile(QString filename, QTcpSocket *socket);
    FileHandler* createFile(QString filename, QTcpSocket *socket);
    void checkLogin(QString username, QString password, QTcpSocket *socket);
    static FileSystem* getInstance();
    std::map<QString, FileHandler*> getFiles();
    void disconnectClient(QTcpSocket* socket);
};

#endif // FILESYSTEM_H

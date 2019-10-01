#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <map>
#include <string>
#include <QString>

class FileHandler;
class QTcpSocket;

class FileSystem
{
    static FileSystem* instance;
    std::map<QString, FileHandler*> files;
    FileSystem() {}
public:
    int sendFile(QString filename, QTcpSocket *socket);
    static FileSystem* getInstance();
    std::map<QString, FileHandler*> getFiles();
};

#endif // FILESYSTEM_H

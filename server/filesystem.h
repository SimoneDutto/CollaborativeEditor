#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <map>
#include <string>

class FileHandler;
class QTcpSocket;

class FileSystem
{
    static FileSystem *instance;
    std::map<std::string, FileHandler> files;
    FileSystem() {}
public:
    FileHandler searchFile(std::string filename);
    int sendFile(std::string filename, QTcpSocket *socket);
    static FileSystem *getInstance();

};

#endif // FILESYSTEM_H

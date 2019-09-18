#include "filehandler.h"

void FileHandler::insertActiveUser(QTcpSocket *user){
    users.append(user);
}

#include "filehandler.h"

void FileHandler::insertActiveUser(QTcpSocket *user){
    active_users.append(user);
}

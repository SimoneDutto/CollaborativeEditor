#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QVector>
#include <QTcpSocket>
#include <QJsonArray>
#include "letter.h"


class FileHandler
{
    QVector<Letter> letters;
    QVector<QTcpSocket*> users;

public:
    FileHandler();
    void remoteInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter);
    void remoteDelete(QString deletedLetterID);
    QVector<QTcpSocket*> active_users;

public:
    FileHandler(QVector<Letter> lecters): letters(lecters){}
    void insertActiveUser(QTcpSocket* user);
    QVector<QTcpSocket*> getUsers();

};

#endif // FILEHANDLER_H

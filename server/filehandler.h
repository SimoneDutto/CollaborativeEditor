#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QVector>
#include <QTcpSocket>
#include <QJsonArray>
#include "letter.h"


class FileHandler
{
    QVector<Letter*> letters;
    QVector<QTcpSocket*> users;

public:

    void remoteInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter);
    void remoteDelete(QString deletedLetterID);
    QVector<QTcpSocket*> active_users;

public:
    FileHandler(const QVector<Letter*>&& lett);
    void insertActiveUser(QTcpSocket* user);
    QVector<QTcpSocket*> getUsers();  

};

#endif // FILEHANDLER_H

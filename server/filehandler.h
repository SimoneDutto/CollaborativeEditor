#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QVector>
#include <QTcpSocket>
#include <QJsonArray>
#include "letter.h"


class FileHandler
{
    QVector<Letter> letters;
    QVector<QTcpSocket> users;

public:
    FileHandler();
    void remoteInsert(QJsonArray position, QChar newLetter, int externalIndex, int siteID, int siteCounter);
    void remoteDelete(QString deletedLetterID);
};

#endif // FILEHANDLER_H

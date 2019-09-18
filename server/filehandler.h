#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QVector>
#include <QTcpSocket>
#include "letter.h"


class FileHandler
{
    QVector<Letter> letters;
    QVector<QTcpSocket*> users;

public:
    FileHandler(QVector<Letter> lecters): letters(lecters){}
    void insertActiveUser(QTcpSocket* user);
};

#endif // FILEHANDLER_H

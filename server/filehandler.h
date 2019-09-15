#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QVector>
#include <QTcpSocket>
#include "letter.h"


class FileHandler
{
    QVector<Letter> lecters;
    QVector<QTcpSocket> users;

public:
    FileHandler();
};

#endif // FILEHANDLER_H

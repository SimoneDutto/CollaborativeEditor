#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QVector>
#include <QTcpSocket>
#include <QJsonArray>
#include <QByteArray>
#include <QObject>
#include "letter.h"


class FileHandler : public QObject
{
    Q_OBJECT
private:
    QVector<Letter*> letters;
    QVector<QTcpSocket*> users;

public:
    void remoteInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter,  QByteArray message);
    void remoteDelete(QString deletedLetterID,  QByteArray message);
    QVector<QTcpSocket*> active_users;

    explicit FileHandler(const QVector<Letter*>&& lett, QObject *parent = nullptr);
    void insertActiveUser(QTcpSocket* user);
    QVector<QTcpSocket*> getUsers();  

signals:
    void remoteInsertNotify(QVector<QTcpSocket*> users, QByteArray message, bool modifiedIndex, int newIndex);
    void remoteDeleteNotify(QVector<QTcpSocket*> users, QByteArray message);

};

#endif // FILEHANDLER_H

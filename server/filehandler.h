#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QVector>
#include <QTcpSocket>
#include <QJsonArray>
#include <QByteArray>
#include <QObject>
#include <QFile>
#include <QJsonDocument>
#include <QMap>
#include <QTextCharFormat>
#include "letter.h"


class FileHandler : public QObject
{
    Q_OBJECT
private:
    QVector<Letter*> letters;
    QVector<QTcpSocket*> users;
    int counter_user = 0;
    int id;
    QMap<QTcpSocket*,int> usersSiteCounters;
    QMap<QTcpSocket*,int> usersCursorPosition;

public:
    explicit FileHandler(const QVector<Letter*>&& lett, int fileid, QObject *parent = nullptr);

    void remoteInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter,  QByteArray message, QTcpSocket *client,
                      QTextCharFormat format, Qt::AlignmentFlag alignment);
    void remoteDelete(QString deletedLetterID,  QByteArray message, QTcpSocket* client, int siteCounter);
    void changeStyle(QString initialIndex, QString lastIndex, QString format, QString font, QTcpSocket* client, QByteArray message);
    void changeAlign(Qt::AlignmentFlag align, QString startID, QString lastID, QTcpSocket* client, QByteArray message);
    void changeCursor(QTcpSocket* client, QByteArray message, int position);
    void changeColor(QString startID, QString lastID, QString color, QTcpSocket* client, QByteArray message);

    void insertActiveUser(QTcpSocket* user, int siteCounter, QString username, int userID, int cursorPosition);
    void removeActiveUser(QTcpSocket *user, QString username, int userID);
    QVector<QTcpSocket*> getUsers();
    QVector<Letter*> getLetter();
    int getSiteCounter(QTcpSocket *user);
    int getUserCursorPosition(QTcpSocket *user);
    bool thereAreUsers();

signals:
    void remoteInsertNotify(QVector<QTcpSocket*> users, QByteArray message, bool modifiedIndex, int newIndex, QTcpSocket *client);
    void remoteDeleteNotify(QVector<QTcpSocket*> users, QByteArray message, QTcpSocket* client);
    void remoteStyleChangeNotify(QVector<QTcpSocket*> users, QByteArray message, QTcpSocket* client);
    void remoteAlignChangeNotify(QVector<QTcpSocket*> users, QByteArray message, QTcpSocket* client);
    void remoteCursorChangeNotify(QVector<QTcpSocket*> users, QByteArray message, QTcpSocket* client);
    void remoteColorChangeNotify(QVector<QTcpSocket*> users, QByteArray message, QTcpSocket* client);

};

#endif // FILEHANDLER_H

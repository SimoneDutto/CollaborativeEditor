#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QObject>
#include <QVector>
#include <QJsonArray>
#include "letter.h"

class FileHandler : public QObject
{
    Q_OBJECT

private:
    int fileid;
    QVector<Letter*> letters;
    int siteCounter;
    int size;
    int cursorPosition;
    QString URI;
    QMap<int,int> usersCursor; /* Mappa <userID, posizioneCursore> */

    QVector<int> calculateInternalIndex(QVector<int> prevPos, QVector<int> nextPos);

public:
    explicit FileHandler(QObject *parent = nullptr);

    QVector<Letter*> getVectorFile();
    int getFileId();
    int getSize();
    int getSiteCounter();
    QString getURI();

    void setValues(QVector<Letter*> letters);
    void setFileId(int fileid);
    void setSize(int size);
    void setSiteCounter(int siteCounter);
    void setURI(QString URI);


public slots:
    void localInsert(int externalIndex, QChar newLetterValue, int clientID, QTextCharFormat format);
    void localDelete(int firstExternalIndex, int lastExternalIndex);
    void localStyleChange(QMap<QString, QTextCharFormat> letterFormatMap, QString startID, QString lastID, bool boldTriggered, bool italicTriggered, bool underlinedTriggered);
    void localCursorPositionChange(int position);
    void remoteInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter, QTextCharFormat format);
    void remoteDelete(QString deletedLetterID);
    void remoteStyleChange(QString firstLetterID, QString lastLetterID, QString changedStyle);
    void remoteCursorPositionChange(int userID, int position);
    void remoteConnectedUser(int userID, int position, QColor color);
    void remoteDisconnectedUser(int userID);

signals:
    void localInsertNotify(QChar newLetterValue, QJsonArray position, int siteID, int siteCounter, int externalIndex, QTextCharFormat format);
    void localDeleteNotify(QString deletedLetterID, int fileid, int siteCounter);
    void localStyleChangeNotify(QString firstLetterID, QString lastLetterID, int fileID, QString changedStyle);
    void localCursorPositionChangeNotify(int fileID, int position);
    void readyRemoteInsert(QChar newLetter, int externalIndex, QTextCharFormat format);
    void readyRemoteDelete(int externalIndex);
    void readyRemoteStyleChange(QString firstLetterID, QString lastLetterID);
    void readyRemoteCursorPositionChange(int userID, int position); // colore?
    void readyRemoteConnectedUser(int userID, int position, QColor color);
    void readyRemoteDisconnectedUser(int userID);
};

#endif // FILEHANDLER_H

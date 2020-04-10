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
    int cursor;
    QString URI;
    QVector<int> calculateInternalIndex(QVector<int> prevPos, QVector<int> nextPos);

public:
    explicit FileHandler(QObject *parent = nullptr);
    void setValues(QVector<Letter*> letters);
    QVector<Letter*> getVectorFile();
    int getFileId();
    void setFileId(int fileid);
    int getSize();
    int getFileSize();
    void setSize(int size);
    int getSiteCounter();
    void setSiteCounter(int siteCounter);
    QString getURI();
    void setURI(QString URI);
    int getCursor();
    void setCursor(int newPosition);

public slots:
    void localInsert(int externalIndex, QChar newLetterValue, int clientID, QTextCharFormat format);
    void localDelete(int firstExternalIndex, int lastExternalIndex);
    void localStyleChange(QMap<QString, QTextCharFormat> letterFormatMap, QString startID, QString lastID, bool boldTriggered, bool italicTriggered, bool underlinedTriggered, QString font);
    void localCursorChange(int position);

    void remoteInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter, QTextCharFormat format);
    void remoteDelete(QString deletedLetterID);
    void remoteStyleChange(QString firstLetterID, QString lastLetterID, QString changedStyle, QString font);
signals:
    void localInsertNotify(QChar newLetterValue, QJsonArray position, int siteID, int siteCounter, int externalIndex, QTextCharFormat format);
    void localDeleteNotify(QString deletedLetterID, int fileid, int siteCounter);
    void localStyleChangeNotify(QString firstLetterID, QString lastLetterID, int fileID, QString changedStyle, QString font);
    void localCursorChangeNotify(int position);

    void readyRemoteInsert(QChar newLetter, int externalIndex, QTextCharFormat format);
    void readyRemoteDelete(int externalIndex);
    void readyRemoteStyleChange(QString firstLetterID, QString lastLetterID);
};

#endif // FILEHANDLER_H

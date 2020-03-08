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
    QString URI;
    QVector<int> calculateInternalIndex(QVector<int> prevPos, QVector<int> nextPos);

public:
    explicit FileHandler(QObject *parent = nullptr);
    void setValues(QVector<Letter*> letters);
    QVector<Letter*> getVectorFile();
    int getFileId();
    void setFileId(int fileid);
    int getSize();
    void setSize(int size);
    int getSiteCounter();
    void setSiteCounter(int siteCounter);
    void setURI(QString URI);
    QString getURI();

public slots:
    void localInsert(int externalIndex, QChar newLetterValue, int clientID, QTextCharFormat format);
    void localDelete(int externalIndex);
    void remoteInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter, QTextCharFormat format);
    void remoteDelete(QString deletedLetterID);
    void localStyleChange(QMap<QString, QTextCharFormat> letterFormatMap);
    void remoteStyleChange(QString firstLetterID, QString lastLetterID, QString changedStyle);

signals:
    void localInsertNotify(QChar newLetterValue, QJsonArray position, int siteID, int siteCounter, int externalIndex, QTextCharFormat format);
    void localDeleteNotify(QString deletedLetterID, int fileid, int siteCounter);
    void localStyleChangeNotify(QString firstLetterID, QString lastLetterID, int fileID, QString changedStyle);
    void readyRemoteInsert(QChar newLetter, int externalIndex, QTextCharFormat format);
    void readyRemoteDelete(int externalIndex);
    void readyRemoteStyleChange(QString firstLetterID, QString lastLetterID);
};

#endif // FILEHANDLER_H

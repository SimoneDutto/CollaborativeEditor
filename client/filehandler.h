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
    QString fileName;
    QVector<Letter*> letters;
    int siteCounter;

    QVector<int> calculateInternalIndex(QVector<int> prevPos, QVector<int> nextPos);
    //void insertLetterInArray(Letter *newLetter);

public:
    explicit FileHandler(QString filename, QObject *parent = nullptr);
    void setVectorLettersFile(QVector<Letter*> letters);
    void setFileName(QString fileName);
    QVector<Letter*> getVectorFile();
    QString getFileName();

public slots:
    void localInsert(int externalIndex, QChar newLetterValue, int clientID);
    void localDelete(int externalIndex);
    void remoteInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter);
    void remoteDelete(QString deletedLetterID);

signals:
    void localInsertNotify(QChar newLetterValue, QJsonArray position, int siteID, int siteCounter, int externalIndex);
    void localDeleteNotify(int externalIndex);
};

#endif // FILEHANDLER_H

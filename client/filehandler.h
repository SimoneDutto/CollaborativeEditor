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
    QVector<QString> listFiles;
    QVector<Letter*> letters;
    int siteCounter;

    QVector<int> calculateInternalIndex(QVector<int> prevPos, QVector<int> nextPos);
    //void insertLetterInArray(Letter *newLetter);

public:
    explicit FileHandler(int siteid, QObject *parent = nullptr);
    void setListFiles(QVector<QString> listFiles);
    void setVectorLettersFile(QVector<Letter*> letters);
    QVector<QString> getListFiles();
    QVector<Letter*> getVectorFile();

public slots:
    void localInsert(int externalIndex, QChar newLetterValue, int clientID);
    void localDelete(int externalIndex);
    void remoteInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter);
    void remoteDelete(QString deletedLetterID);

signals:
    void localInsertNotify(int pos, QString value);
    void localDeleteNotify(int externalIndex);
};

#endif // FILEHANDLER_H

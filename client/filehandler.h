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

    QVector<int> calculateInternalIndex(QVector<int> prevPos, QVector<int> nextPos);

public:
    explicit FileHandler(QObject *parent = nullptr);
    void setValues(QVector<Letter*> letters, int fileid);
    QVector<Letter*> getVectorFile();
    int getFileId();

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

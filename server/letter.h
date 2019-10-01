#ifndef LECTER_H
#define LECTER_H

#include <QVector>

class Letter
{
    // campi per gestire la lettera, la posizione, lo stile, etc
private:
    QChar letter;
    //int siteID;
    //int counter;
    QVector<int> fractionalIndexes;
    QString letterID;

public:
    //Letter(int socketID);
    Letter(QChar letter, QVector<int> fractionals, QString letterID);
    QChar getLetterValue();
    QVector<int> getFractionalIndexes();
    QString getLetterID();
    int getSiteID();
    int getIndex();
    int getNumberOfFractionals();
    //int getCounter();
    //void incrementCounter();
    void setIndex(int index);
    void addFractionalDigit(int value);
    bool hasSameFractionals(Letter other);
    bool comesFirst(Letter other);
};

#endif // LECTER_H

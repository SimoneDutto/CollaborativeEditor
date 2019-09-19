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

public:
    //Letter(int socketID);
    Letter(QChar letter, int index, QVector<int> fractionals);
    QChar getLetterValue();
    QVector<int> getFractionalIndexes();
    int getIndex();
    //int getCounter();
    //void incrementCounter();
    void setIndex(int index);
    void addFractionalDigit(int value);
    QChar getLetterID();
};

#endif // LECTER_H

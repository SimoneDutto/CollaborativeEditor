#ifndef LECTER_H
#define LECTER_H
#include <QString>
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
    Letter() {}
    Letter(QChar letter, QVector<int> fractionals, QString letterID);
    //Letter(const Letter& letter);
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

#ifndef LECTER_H
#define LECTER_H

#include <QString>
#include <QVector>
#include <QChar>

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
    Letter(QString value = QString(), QString id  = QString(), int intera = 0, int decimale = 0);
    //Letter() {}
    Letter(QChar letter, QVector<int> fractionals, QString letterID);
    QChar getValue();
    QVector<int> getFractionalIndexes();
    QString getLetterID();
    int getSiteID();
    int getIndex();
    int getNumberOfFractionals();
    void setIndex(int index);
    void editIndex(int index, int value);
    void addFractionalDigit(int value);
    bool hasSameFractionals(Letter other);
    bool comesFirst(Letter other);
};

#endif // LECTER_H

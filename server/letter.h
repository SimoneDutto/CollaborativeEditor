#ifndef LECTER_H
#define LECTER_H
#include <QString>
#include <QVector>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>

class Letter
{
    // campi per gestire la lettera, la posizione, lo stile, etc
public:
    enum Styles {Normal, Bold, Italic, Underlined};
private:
    QChar letter;
    //int siteID;
    //int counter;
    QVector<int> fractionalIndexes;
    QString letterID;
    Styles style;

public:
    //Letter() {}
    Letter(QChar letter, QVector<int> fractionals, QString letterID);
    //Letter(const Letter& letter);
    QChar getLetterValue();
    QVector<int> getFractionalIndexes();
    QString getLetterID();
    int getSiteID();
    int getIndex();
    int getNumberOfFractionals();
    void setStyle(QString style);
    //int getCounter();
    //void incrementCounter();
    void setIndex(int index);
    void addFractionalDigit(int value);
    bool hasSameFractionals(Letter other);
    bool comesFirst(Letter other);
    QJsonObject toJSon();

};

#endif // LECTER_H

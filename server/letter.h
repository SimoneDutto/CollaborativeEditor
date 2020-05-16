#ifndef LECTER_H
#define LECTER_H
#include <QString>
#include <QVector>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextCharFormat>

class Letter
{
    // campi per gestire la lettera, la posizione, lo stile, etc
public:
    enum Styles {Normal, Italic, Bold, Underlined, NotItalic, NotBold, NotUnderlined};
private:
    QChar letter;
    //int siteID;
    //int counter;
    QVector<int> fractionalIndexes;
    QString letterID;
    QTextCharFormat format;

public:
    //Letter() {}
    Letter(QChar letter, QVector<int> fractionals, QString letterID, QTextCharFormat format);
    //Letter(const Letter& letter);
    QChar getLetterValue();
    QVector<int> getFractionalIndexes();
    QString getLetterID();
    int getSiteID();
    int getIndex();
    int getNumberOfFractionals();
    QTextCharFormat getFormat();
    void setStyleFromString(QString format, QString font);
    //int getCounter();
    //void incrementCounter();
    void setIndex(int index);
    void addFractionalDigit(int value);
    bool hasSameFractionals(Letter other);
    bool comesFirst(Letter other);
    QJsonObject toJSon();

};

#endif // LECTER_H

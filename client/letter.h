#ifndef LECTER_H
#define LECTER_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QChar>

class Letter
{
    // campi per gestire la lettera, la posizione, lo stile, etc
private:
    QChar letter;
    QVector<int> fractionalIndexes;
    QString letterID;
    bool isBold;
    bool isUnderlined;
    bool isItalic;

public:
    Letter(QChar letter, QVector<int> fractionals, QString letterID);

    //Aggiornamento stili
    Letter(QChar letter, QVector<int> fractionals, QString letterID, bool isBold, bool isUnderlined, bool isItalic);

    Letter(const Letter& l);
    Letter& operator=(const Letter && source);
    Letter& operator=(const Letter& source);
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
    void setBoldBool(bool value);
    void setUnderlinedBool(bool value);
    void setItalicBool(bool value);
    bool getBoldBool();
    bool getUnderlinedBool();
    bool getItalicBool();
    ~Letter(){}
};

#endif // LECTER_H

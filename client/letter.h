#ifndef LECTER_H
#define LECTER_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QChar>
#include <QTextCursor>

class Letter
{
    // campi per gestire la lettera, la posizione, lo stile, etc
private:
    QChar letter;
    QVector<int> fractionalIndexes;
    QString letterID;
    QTextCharFormat format;
    Qt::AlignmentFlag alignment;
public:
    Letter(QChar letter, QVector<int> fractionals, QString letterID, QTextCharFormat format);   // AGGIUNGERE ALIGNMENT
    Letter(const Letter& l);
    Letter& operator=(const Letter && source);
    Letter& operator=(const Letter& source);
    QChar getValue();
    QVector<int> getFractionalIndexes();
    QString getLetterID();
    int getSiteID();
    int getIndex();
    int getUserId();
    int getNumberOfFractionals();
    QTextCharFormat getFormat();
    void setIndex(int index);
    void setStyle(QString style);
    void editIndex(int index, int value);
    void addFractionalDigit(int value);
    bool hasSameFractionals(Letter other);
    bool comesFirst(Letter other);
    void setFormat(QTextCharFormat format);
    void setStyleFromString(QString format, QString font);
    void setAlignment(Qt::AlignmentFlag alignment);
    void setColor(QColor c);
    void setBack(QColor c);
    QBrush getColor();
    QBrush getBack();
    ~Letter(){}
};

#endif // LECTER_H

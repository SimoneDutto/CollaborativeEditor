#ifndef LECTER_H
#define LECTER_H

#include <QString>
#include <QVector>

class Letter
{
    // campi per gestire la lettera, la posizione, lo stile, etc
private:
    QString value;
    QString id;
    QVector<int> pos;

public:
    Letter(QString value = QString(), QString id  = QString(), int intera = 0, int decimale = 0);
    QVector<int> getPos();
    QString getId();
    QString getValue();
};

#endif // LECTER_H

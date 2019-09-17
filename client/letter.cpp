#include "letter.h"

Letter::Letter(QString value, QString id, int intera, int decimale):
    value(value), id(id)
{
    this->pos.append(intera);
    this->pos.append(decimale);
}

QVector<int> Letter::getPos()
{
    return this->pos;
}

QString Letter::getId()
{
    return this->id;
}

QString Letter::getValue()
{
    return this->value;
}

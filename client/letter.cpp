#include "letter.h"

Letter::Letter(QString value, QString id, int intera, int decimale):
    letterID(id)
{
    this->letter = value.at(0);
    this->fractionalIndexes.insert(0, intera);
    this->fractionalIndexes.insert(1, decimale);
}
/*
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
*/

Letter::Letter(QChar letter, QVector<int> fractionals, QString letterID) : letter(letter), fractionalIndexes(fractionals), letterID(letterID) {
    //this->fractionalIndexes.insert(0, index);
    // index già presente nella posizione 0 di fractionals
}

QChar Letter::getValue() {
    return this->letter;
}

QVector<int> Letter::getFractionalIndexes() {
    return this->fractionalIndexes;
}

int Letter::getIndex() {
    return this->fractionalIndexes.at(0);
}

void Letter::setIndex(int index) {
    this->fractionalIndexes.replace(0, index);
}

void Letter::editIndex(int index, int value) {
    this->fractionalIndexes.replace(index, value);
}

QString Letter::getLetterID() {
    return this->letterID;
}

int Letter::getSiteID() {
    int siteID = this->letterID.split("-")[0].toInt();
    return siteID;
}

int Letter::getNumberOfFractionals() {
    return this->fractionalIndexes.size();
}

void Letter::addFractionalDigit(int value) {
    this->fractionalIndexes.append(value);
}

bool Letter::hasSameFractionals(Letter other) {
    QVector<int> otherFractionals = other.getFractionalIndexes();

    if(this->fractionalIndexes.size() == otherFractionals.size()) {
        for(int i=0; i<otherFractionals.size(); i++) {
            if(this->fractionalIndexes.at(i) != otherFractionals.at(i))
                return false;
        }
        return true;
    } else return false;
}

bool Letter::comesFirst(Letter other) {
    bool comesFirst = false;

    if(this->getIndex() < other.getIndex())
        comesFirst = true;
    else {
        if(this->getNumberOfFractionals() < other.getNumberOfFractionals())
            comesFirst = true;
        else if(this->getNumberOfFractionals() == other.getNumberOfFractionals()) {
            for(int i=0; i<this->getNumberOfFractionals(); i++) {
                if(this->fractionalIndexes[i] < other.getFractionalIndexes()[i]) {
                    comesFirst = true;
                    break;
                }
            }
        }
    }
    return comesFirst;
}

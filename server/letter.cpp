#include "letter.h"

Letter::Letter(QChar letter, int index, QVector<int> fractionals, QString letterID)
{
    this->letter = letter;
    this->fractionalIndexes.insert(0, index);
    for(int fractional : fractionals)
        this->fractionalIndexes.append(fractional);
    this->letterID = letterID;
    //this->fractionals.insert(0, 0); // first fractional digit: 0
}

/*int Letter::getCounter() {
    return this->counter;
}

void Letter::incrementCounter() {
    this->counter++;
}

*/

QChar Letter::getLetterValue() {
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
    bool equals = true;
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



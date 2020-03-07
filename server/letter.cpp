#include "letter.h"

Letter::Letter(QChar letter, QVector<int> fractionals, QString letterID, QTextCharFormat format) :
    letter(letter),
    fractionalIndexes(fractionals),
    letterID(letterID),
    format(format) {}

/*Letter::Letter(const Letter& other) {
    this->letter = other.letter;
    this->letterID = other.letterID;
    for(auto f : other.fractionalIndexes)
       this->fractionalIndexes.append(f);
}*/


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

/*void Letter::setStyle(QString style) {
    if(style.compare("Normal") == 0)
        this->style = Normal;
    else if(style.compare("Bold") == 0)
        this->style = Bold;
    else if(style.compare("Italic") == 0)
        this->style = Italic;
    else if(style.compare("Underlined") == 0)
        this->style = Underlined;
}*/

QTextCharFormat Letter::getFormat() {
    return this->format;
}

void Letter::setStyleFromString(QString format) {
    if(format.compare("Bold") == 0)
        this->format.setFontWeight(75);
    else if(format.compare("NotBold") == 0)
        this->format.setFontWeight(50);
    else if(format.compare("Italic") == 0)
        this->format.setFontItalic(true);
    else if(format.compare("NotItalic") == 0)
        this->format.setFontItalic(false);
    else if(format.compare("Underlined") == 0)
        this->format.setFontUnderline(true);
    else if(format.compare("Underlined") == 0)
        this->format.setFontUnderline(false);
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

QJsonObject Letter::toJSon(){
    QJsonArray positionJsonArray;
    std::copy (fractionalIndexes.begin(), fractionalIndexes.end(), std::back_inserter(positionJsonArray));
    QJsonObject obj;
    obj.insert("letter", QJsonValue(letter));
    obj.insert("letterID", QJsonValue(letterID));
    obj.insert("isBold", QJsonValue(format.fontWeight()==75));
    obj.insert("isItalic", QJsonValue(format.fontItalic()));
    obj.insert("isUnderlined", QJsonValue(format.fontUnderline()));
    obj.insert("position", positionJsonArray);

    return obj;
}



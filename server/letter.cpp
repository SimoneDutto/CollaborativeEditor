#include "letter.h"
#include <qdebug.h>

Letter::Letter(QChar letter, QVector<int> fractionals, QString letterID, QTextCharFormat format, Qt::AlignmentFlag alignment) :
    letter(letter),
    fractionalIndexes(fractionals),
    letterID(letterID),
    format(format),
    alignment(alignment) {}

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

void Letter::setStyleFromString(QString format, QString font) { // TODO togliere format
    QFont f;
    f.fromString(font);
    this->format.setFont(f, QTextCharFormat::FontPropertiesSpecifiedOnly);
    // qDebug() << "font cambiato" << this->format.font();
}

void Letter::setAlignment(Qt::AlignmentFlag align) {
    this->alignment = align;
    qDebug() << "alignment changed";
}

void Letter::setColor(QColor c) {
    this->format.setForeground(c);
}

QBrush Letter::getColor() {
    return this->format.foreground();
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

/*bool Letter::comesFirst(Letter other) {
    bool comesFirst = false;

    if(this->getIndex() < other.getIndex())
        comesFirst = true;
    else if(this->getIndex() > other.getIndex())
        comesFirst = false;
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
        } else {

            for(int i=0; i<other.getNumberOfFractionals(); i++) {
                if(this->fractionalIndexes[i] < other.getFractionalIndexes()[i]) {
                    comesFirst = true;
                    break;
                }
            }
        }
    }
    return comesFirst;
}*/

bool Letter::comesFirstRight(Letter other, int pos_id) {
    int this_nfractionals = this->getNumberOfFractionals(), other_nfractionals = other.getNumberOfFractionals();

    //qDebug() << this->fractionalIndexes.at(pos_id) << other.getFractionalIndexes().at(pos_id);
    if(this->fractionalIndexes.at(pos_id) < other.getFractionalIndexes().at(pos_id))
        return true;
    else if(this->fractionalIndexes.at(pos_id) > other.getFractionalIndexes().at(pos_id))
        return false;
    else {
        if(other_nfractionals > pos_id+1 && this_nfractionals <= pos_id+1)
            return true;
        else if (other_nfractionals <= pos_id+1 && this_nfractionals > pos_id+1)
            return false;
        else if (other_nfractionals > pos_id+1 && this_nfractionals > pos_id+1)
            return this->comesFirstRight(other, pos_id+1);
        else {
            QStringList this_ids = this->letterID.split("-"), other_ids = other.letterID.split("-");
            int this_id = this_ids.at(0).toInt(), this_cnt = this_ids.at(1).toInt(), other_id = other_ids.at(0).toInt(), other_cnt = other_ids.at(1).toInt();
            if(this_id < other_id || (!(this_id < other_id) && this_cnt < other_cnt))
                return true;
            else return false;
        }
    }
}

bool Letter::comesFirstLeft(Letter other, int pos_id) {
    int this_nfractionals = this->getNumberOfFractionals(), other_nfractionals = other.getNumberOfFractionals();

    if(this->fractionalIndexes.at(pos_id) < other.getFractionalIndexes().at(pos_id))
        return false;
    else if(this->fractionalIndexes.at(pos_id) > getFractionalIndexes().at(pos_id))
        return true;
    else {
        if(other_nfractionals > pos_id+1 && this_nfractionals <= pos_id+1)
            return false;
        else if (other_nfractionals <= pos_id+1 && this_nfractionals > pos_id+1)
            return true;
        else if (other_nfractionals > pos_id+1 && this_nfractionals > pos_id+1)
            return this->comesFirstLeft(other, pos_id+1);
        else {
            QStringList this_ids = this->letterID.split("-"), other_ids = other.letterID.split("-");
            int this_id = this_ids.at(0).toInt(), this_cnt = this_ids.at(1).toInt(), other_id = other_ids.at(0).toInt(), other_cnt = other_ids.at(1).toInt();
            if(this_id < other_id || (!(this_id < other_id) && this_cnt < other_cnt))
                return false;
            else return true;
        }
    }
}

void Letter::setNewFractionals(QVector<int> newFractionals) {
    this->fractionalIndexes.clear();
    for(int pos : newFractionals)
        this->fractionalIndexes.append(pos);
}

QJsonObject Letter::toJSon(){
    QJsonArray positionJsonArray;
    std::copy (fractionalIndexes.begin(), fractionalIndexes.end(), std::back_inserter(positionJsonArray));
    QJsonObject obj;
    obj.insert("letter", QJsonValue(letter));
    obj.insert("letterID", QJsonValue(letterID));
    obj.insert("font", QJsonValue(format.font().toString()));
    obj.insert("align", QJsonValue(alignment));
    obj.insert("color", QJsonValue(format.foreground().color().name()));
    obj.insert("position", positionJsonArray);
    //qDebug() << "TO JSON" << alignment;

    return obj;
}



#include "letter.h"
#include <QDebug>

Letter::Letter(QChar letter, QVector<int> fractionals, QString letterID, QTextCharFormat format, Qt::AlignmentFlag alignment)
    : letter(letter),
      fractionalIndexes(fractionals),
      letterID(letterID),
      format(format),
      alignment(alignment) {}

// Costruttore di copia
Letter::Letter(const Letter& l) {
    this->letter = std::move(l.letter);
    this->letterID = std::move(l.letterID);
    this->fractionalIndexes.append(std::move(l.fractionalIndexes));
    this->alignment = std::move(l.alignment);
    this->format = std::move(l.format);
}

// Overload operatore di assegnazione
Letter& Letter::operator=(const Letter& source) {
    if(this != &source) {
        this->letter = std::move(source.letter);
        this->letterID = std::move(source.letterID);
        this->format = std::move(source.format);
        this->alignment = std::move(source.alignment);
        this->fractionalIndexes.erase(this->fractionalIndexes.begin(), this->fractionalIndexes.end());
        this->fractionalIndexes.append(std::move(source.fractionalIndexes));
    }
    return *this;
}

// Overload operatore di movimento
Letter& Letter::operator=(const Letter && source) {
    if(this != &source) {
        this->letter = std::move(source.letter);
        this->letterID = std::move(source.letterID);
        this->format = std::move(source.format);
        this->alignment = std::move(source.alignment);
        this->fractionalIndexes.erase(this->fractionalIndexes.begin(), this->fractionalIndexes.end());
        this->fractionalIndexes.append(std::move(source.fractionalIndexes));
    }
    return *this;
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

void Letter::setStyle(QString style) {
    if(style.compare("Bold")==0)
        format.setFontWeight(75);
    else if(style.compare("NotBold")==0)
        format.setFontWeight(50);
    else if(style.compare("Italic")==0)
        format.setFontItalic(true);
    else if(style.compare("NotItalic")==0)
        format.setFontItalic(false);
    else if(style.compare("Underlined")==0)
        format.setFontUnderline(true);
    else if(style.compare("NotUnderlined")==0)
        format.setFontUnderline(false);
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

bool Letter::comesFirstRight(Letter other, int pos_id) {
    int this_nfractionals = this->getNumberOfFractionals(), other_nfractionals = other.getNumberOfFractionals();

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
        /*if(this->getNumberOfFractionals() < other.getNumberOfFractionals())
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
        }*/
    }
}

bool Letter::comesFirstLeft(Letter other, int pos_id) {
    int this_nfractionals = this->getNumberOfFractionals(), other_nfractionals = other.getNumberOfFractionals();

    if(this->fractionalIndexes.at(pos_id) < other.getFractionalIndexes().at(pos_id))
        return false;
    else if(this->fractionalIndexes.at(pos_id) > other.getFractionalIndexes().at(pos_id))
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


void Letter::setNewPosition(QVector<int> position) {
    this->fractionalIndexes.clear();
    for(int pos : position)
        this->fractionalIndexes.append(pos);
}
void Letter::setFormat(QTextCharFormat format){
    this->format=format;
}

void Letter::setStyleFromString(QString format, QString font) {
    QFont f;
    f.fromString(font);
    qDebug() << "SET" << this->getFormat().font() << f.toString() << font;
    this->format.setFont(f);
    qDebug() << this->getFormat().font();
}

void Letter::setAlignment(Qt::AlignmentFlag alignment) {
    this->alignment = alignment;
}

int Letter::getUserId(){
    return this->letterID.split("-").at(0).toInt();
}

QTextCharFormat Letter::getFormat(){
    qDebug() <<"Letter format:"  << this->format.font().toString() << " " << this->format.fontItalic();
    return this->format;
}

void Letter::setColor(QColor c)
{
    this->format.setForeground(c);
}

void Letter::setBack(QColor c){
    this->format.setBackground(c);
}

QBrush Letter::getColor()
{
    return this->format.foreground();
}

QBrush Letter::getBack(){
    return this->format.background();
}

Qt::AlignmentFlag Letter::getAlignment() {
    return this->alignment;
}

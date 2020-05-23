#include "filehandler.h"
#include <QDebug>

FileHandler::FileHandler(QObject *parent)
  : QObject(parent), siteCounter(0), cursor(0)
{}

/**
  0. Incrementa contatore delle modifiche sul file (siteCounter)
  1. Se la lettera inserita si trova alla fine del file, l'indice interno corrisponderà al primo indice disponibile (lastIndex+1)
  2. Altrimenti, calcola l'indice interno intero e frazionario, accedendo al vettore di lettere del file:
    2.1 Calcolo indice intero:
        Se la lettera è stata inserita all'inizio del file, indice interno intero = 0 (nextPos.at(0))
        Altrimenti, indice interno intero è uguale a quello della lettera precedente (prevPos.at(0))
    2.2 Calcolo indici frazionari:
        Se la lettera precedente ha indici frazionari, questi vengono copiati nel vettore di posizioni della nuova lettera inserita, tranne l'ultimo
        Se non li ha, viene considerato l'ultimo indice frazionario pari a 0
    2.3 Calcolo ultimo indice frazionario (determina la posizione differente rispetto alle lettere circostanti):
        Se gli indici interi delle lettere a dx e sx sono diversi, il nuovo indice frazionario è (INT_MAX-ultimoIndiceLetteraPrecedente): se risultano uguali,
        viene aggiunta una nuova posizione frazionaria, pari a INT_MAX/2
        Altrimenti, se sono uguali, il nuovo indice è la media tra i due a dx e sx: se risulta uguale a 0 o uguale all'indice precedente, viene aggiutna una nuova posizione
        frazionaria, pari a INT_MAX/2.
*/


QVector<int> FileHandler::calculateInternalIndex(QVector<int> prevPos, QVector<int> nextPos) {
    QVector<int> position;

    // Set internal index
    if(prevPos.isEmpty()) { // externalIndex == 0
        position.insert(0, nextPos.at(0));
    } else {
        position.insert(0, prevPos.at(0));  // indice intero pari a quello della lettera che precede

        int lastFractionalPrev, lastFractionalNext, newIndex;
        if(prevPos.size() > 1) {    // indici frazionari presenti
            qDebug() << "prePos.size() > 1";
            lastFractionalPrev = prevPos[prevPos.size()-1];
            if(prevPos.size() > 2) {    // copia gli indici frazionari della lettera precedente
                for(int i=1; i<prevPos.size(); i++) // size-1?
                    position.append(prevPos[i]);
            }
        } else lastFractionalPrev = 0;

        if(nextPos.size() > 1) {
            lastFractionalNext = nextPos[nextPos.size()-1];
        } else lastFractionalNext = 0;

        if(prevPos.at(0) < nextPos.at(0)) { // indici interi diversi
            if(lastFractionalNext == 0 && lastFractionalPrev == 0) {
                newIndex = int(INT_MAX/2);
            } else {
                newIndex = int(INT_MAX/2 + lastFractionalPrev/2);    // media
                qDebug() << "New = " << newIndex << ", last = " << lastFractionalPrev;
            }

            if(newIndex == lastFractionalPrev) {    // || newIndex == INT_MAX
                qDebug() << "Doppia append";
                position.append(lastFractionalPrev);
                position.append(int(INT_MAX/2));
            }
            else position.append(newIndex);
        } else {    // indici uguali
            newIndex = int(lastFractionalNext/2 + lastFractionalPrev/2);

            if(newIndex == 0 || newIndex == lastFractionalPrev) {
                position.append(lastFractionalPrev);
                position.append(int(INT_MAX/2));
            } else
                position.append(newIndex);
        }
    }
    qDebug() << "Position calculated: ";
    for (auto p : position)
        qDebug() << p <<"-";

    return position;
}

void FileHandler::localInsert(int externalIndex, QChar newLetterValue, int clientID, QTextCharFormat format) {

    int lastIndex = 0;
    qDebug() << "Calcolo l'indice della lettera inserita localmente...";

    if(this->letters.size() > 0) {
        Letter *lastLetter = this->letters.at(letters.size()-1);
        lastIndex = lastLetter->getIndex();
    }

    qDebug() << "Indice dell'ultima lettera del file: " << lastIndex;

    QVector<int> position, previousLetterPos, nextLetterPos;
    int internalIndex = -1;

    this->siteCounter++;
    qDebug() << "Site counter = " << siteCounter;

    QString letterID = QString::number(clientID).append("-").append(QString::number(this->siteCounter));

    if(externalIndex > this->letters.size()) {
        // la lettera inserita si trova alla fine del file
        if(this->letters.size() == 0) // caso prima lettera inserita
            internalIndex = 0;
        else
            internalIndex = lastIndex+1;
        position.insert(0, internalIndex);  //position = {internalIndex}
        qDebug() << "Lettera inserita alla fine del file in posizione " << internalIndex;
    } else {
        if(externalIndex > 1)   // lettera NON inserita all'inizio del file
            previousLetterPos = this->letters[externalIndex-2]->getFractionalIndexes();
        nextLetterPos = this->letters[externalIndex-1]->getFractionalIndexes();

        position = calculateInternalIndex(previousLetterPos, nextLetterPos);
        if(position.size() == 1 && position.at(0) == 0) {   // position = {0}
            //  Lettera inserita all'inizio del file: avrà indici {0}. Devo modificare la lettera che inizialmente aveva questi indici
            int last, value;
            bool sizeIsOne = false;
            if(nextLetterPos.size() == 1) {
                last = 1;
                value = this->letters[0]->getFractionalIndexes()[0];
                sizeIsOne = true;
            } else {
                last = nextLetterPos.size()-1;  // ultimo indice valido di nextLetterPos
                value = this->letters[0]->getFractionalIndexes()[last];  // prendo ultimo frazionario della lettera all'inizio del file
            }

            if(value < INT_MAX) {
                if(sizeIsOne)
                    this->letters[0]->addFractionalDigit(1);
                else
                    this->letters[0]->editIndex(last, value+1);
                // check che la seconda lettera non abbia gli stessi indici dopo la modifica
                if(this->letters.size() >= 2) {
                    if(this->letters[0]->hasSameFractionals(*this->letters[1])) {
                        this->letters[0]->editIndex(last, value);
                        this->letters[0]->addFractionalDigit(INT_MAX/2);
                    }
                }

            } else
                this->letters[0]->addFractionalDigit(INT_MAX/2);
        }
    }

    Letter *newLetter = new Letter(newLetterValue, position, letterID, format);

    qDebug() << "Letter inserted in position:" << position << " (external index " << externalIndex <<")";
    this->letters.insert(this->letters.begin()+(externalIndex-1), newLetter);

    /*Inviare notifica via socket*/
    QJsonArray positionJsonArray;
    std::copy (position.begin(), position.end(), std::back_inserter(positionJsonArray));

    emit localInsertNotify(newLetterValue, positionJsonArray, clientID, siteCounter, externalIndex, format);
}

void FileHandler::localDelete(int firstExternalIndex, int lastExternalIndex) {
    qDebug() << "Removing letter from index " << firstExternalIndex << "to index " << lastExternalIndex;
   //QString letterIDs[lastExternalIndex-firstExternalIndex+1]; // numero di lettere cancellate

    for(int i=lastExternalIndex; i>=firstExternalIndex; i--) {
        qDebug() << "size =" << this->letters.size();
        QString letterID = this->letters[i-1]->getLetterID();
        //letterIDs->append(letterID);
        this->letters.remove(i-1);
        this->siteCounter++;
        /*Inviare notifica via socket*/
        emit localDeleteNotify(letterID, this->fileid, this->siteCounter);
    }
}

void FileHandler::localStyleChange(QMap<QString,QTextCharFormat> letterFormatMap) {
    QString changedStyle;
    QString fontString;
    /* Edit letters style locally */
    for(Letter *l : this->letters) {
        if(letterFormatMap.contains(l->getLetterID())) {
            // Look for the change
            fontString = letterFormatMap.take(l->getLetterID()).font().toString();
            l->setStyleFromString("", letterFormatMap.take(l->getLetterID()).font().toString());
            /* Send change to server */
            emit localStyleChangeNotify(l->getLetterID(), l->getLetterID(), this->fileid, changedStyle, fontString);
        }
    }
}

void FileHandler::localCursorChange(int position) {
    this->cursor = position;
    /* Notify server */
    emit localCursorChangeNotify(position);
}

void FileHandler::localAlignChange(Qt::AlignmentFlag alignment, int cursorPosition, QString startID, QString lastID) {
    bool intervalStarted = false;
    /* Store alignment information for each letter locally */
    for(Letter *l : this->letters) {
        if(!intervalStarted && l->getLetterID().compare(startID)==0) {
            intervalStarted = true;
            l->setAlignment(alignment);
        } else if(intervalStarted) {
            l->setAlignment(alignment);
            if(l->getLetterID().compare(lastID)==0)
                break;
        }
    }
    /* Send change to server */
    emit localAlignChangeNotify(alignment, cursorPosition, startID, lastID);
}

void FileHandler::remoteInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter, QTextCharFormat format) {

    // Get index and fractionals vector
    QVector<int> fractionals;

    if(!position.isEmpty()) {
        //int index = position.at(0).toInt();
        //position.removeAt(0);

        for(auto fractional : position) {
            fractionals.append(fractional.toInt());
        }

        QString letterID = QString::number(siteID).append("-").append(QString::number(siteCounter));

        this->letters.insert(this->letters.begin()+externalIndex-1, new Letter(newLetterValue, fractionals, letterID, format));
    }

    /*Aggiornare la GUI*/
    emit readyRemoteInsert(newLetterValue, externalIndex-1, format);
}

void FileHandler::remoteDelete(QString deletedLetterID) {
    int externalIndex = 0;

    for (Letter *l : this->letters) {
        if(l->getLetterID().compare(deletedLetterID) == 0) {
            this->letters.remove(externalIndex);
            break;
        }
       externalIndex++;
    }

    externalIndex++;    // align externalIndex with GUI rapresentation

    /*Aggiornare la GUI*/
    emit readyRemoteDelete(externalIndex);
}


void FileHandler::remoteStyleChange(QString firstLetterID, QString lastLetterID, QString changedStyle, QString font) {
    /* Edit letters style locally */
    bool intervalStarted = false;

    for(Letter *l : this->letters) {
        if(l->getLetterID().compare(firstLetterID) == 0 || intervalStarted) {
            intervalStarted = true;
            l->setStyleFromString(changedStyle, font);
            if(l->getLetterID().compare(lastLetterID) == 0)
                break;
        }
    }

    /* Aggiornare la GUI */
    emit readyRemoteStyleChange(firstLetterID, lastLetterID);
}

void FileHandler::remoteAlignChange(Qt::AlignmentFlag alignment, int cursorPosition, QString startID, QString lastID) {
    bool intervalStarted = false;
    /* Store alignment information for each letter locally */
    for(Letter *l : this->letters) {
        if(!intervalStarted && l->getLetterID().compare(startID)==0) {
            intervalStarted = true;
            l->setAlignment(alignment);
        } else if(intervalStarted) {
            l->setAlignment(alignment);
            if(l->getLetterID().compare(lastID)==0)
                break;
        }
    }

    /* Update GUI */
    emit readyRemoteAlignChange(alignment, cursorPosition);
}

void FileHandler::setValues(QVector<Letter *> letters){
    if(!this->letters.empty()){
        this->letters.clear();
    }
    this->letters = letters;
    this->cursor = letters.size();
}

QVector<Letter*> FileHandler::getVectorFile(){
    return this->letters;
}

int FileHandler::getFileId(){
    return this->fileid;
}

int FileHandler::getFileSize(){
    return this->letters.size();
}

void FileHandler::setFileId(int fileid){
    this->fileid = fileid;
}

int FileHandler::getSize(){
    return this->size;
}

void FileHandler::setSize(int size){
    this->size = size;
}

int FileHandler::getSiteCounter() {
    return this->siteCounter;
}

void FileHandler::setSiteCounter(int siteCounter){
    this->siteCounter = siteCounter;
}

void FileHandler::setURI(QString URI){
    this->URI = URI;
}

QString FileHandler::getURI(){
    return this->URI;
}

int FileHandler::getCursor() {
    return this->cursor;
}

void FileHandler::setCursor(int newPosition) {
    this->cursor = newPosition;
}

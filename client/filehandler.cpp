#include "filehandler.h"
#include <QDebug>
#include <QThread>

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

QVector<int> generatePositionBetween(QVector<int> previous, QVector<int> next, QVector<int> new_pos) {
    int pos1, pos2;
    // Indici interi delle posizioni
    pos1 = previous.at(0);
    pos2 = next.at(0);

    if(pos1 - pos2 == 0) {  // indici uguali -> indice frazionario
        new_pos.append(pos1);
        previous.remove(0);
        next.remove(0);
        if(previous.isEmpty()) {
            new_pos.append(next.front()-1);
            return new_pos;
        }
        return generatePositionBetween(previous, next, new_pos);
    } else if (pos2 - pos1 == 1) {  // indici consecutivi
        new_pos.append(pos1);
        previous.remove(0);
        if(previous.isEmpty()) {
            new_pos.append(0);
            return new_pos;
        }
        new_pos.append(previous.front()+1);
        return new_pos;
    } else if (pos2 - pos1 > 1) {
        new_pos.append(previous.front()+1);
        return new_pos;
    }
}

void FileHandler::localInsert(int externalIndex, QChar newLetterValue, int clientID, QTextCharFormat format, Qt::AlignmentFlag alignment) {
    QVector<int> position;

    this->siteCounter++;
    QString letterID = QString::number(clientID).append("-").append(QString::number(this->siteCounter));

    if(this->letters.size() == 0) {
        position.append(0);
    } else if (externalIndex > this->letters.size()) {  // la lettera inserita si trova alla fine del file
        Letter *lastLetter = this->letters.at(letters.size()-1);
        int lastIndex = lastLetter->getIndex();
        position.append(lastIndex+1);
    } else if (externalIndex == 1) {    // la lettera inserita si trova all'inizio del file
        int firstIndex = this->letters.at(0)->getIndex();
        position.append(firstIndex-1);  // può essere negativo
    } else {    // la lettera inserita si trova in mezzo al file
        QVector<int> previousLetterPos, nextLetterPos;
        previousLetterPos = this->letters.at(externalIndex-2)->getFractionalIndexes();
        nextLetterPos = this->letters.at(externalIndex-1)->getFractionalIndexes();
        position = generatePositionBetween(previousLetterPos, nextLetterPos, position);
    }

    Letter *newLetter = new Letter(newLetterValue, position, letterID, format, alignment);
    this->letters.insert(this->letters.begin()+(externalIndex-1), newLetter);
    qDebug() << "Position calculated: ";
    for (auto p : position)
        qDebug() << p <<"-";

    // Inviare notifica al server
    emit localInsertNotify(newLetter, clientID, siteCounter, externalIndex, false, nullptr);
}

void FileHandler::localDelete(int firstExternalIndex, int lastExternalIndex) {
    qDebug() << "Removing letter from index " << firstExternalIndex << "to index " << lastExternalIndex;
   //QString letterIDs[lastExternalIndex-firstExternalIndex+1]; // numero di lettere cancellate

    for(int i=lastExternalIndex; i>=firstExternalIndex; i--) {
        qDebug() << "size =" << this->letters.size();
        QString letterID = this->letters[i-1]->getLetterID();
        this->letters.remove(i-1);
        this->siteCounter++;
        /*Inviare notifica via socket*/
        QThread::msleep(1);
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
            l->setStyleFromString("", fontString);
            /* Send change to server */
            emit localStyleChangeNotify(l->getLetterID(), l->getLetterID(), this->fileid, changedStyle, fontString);
        }
    }
}

void FileHandler::localCursorChange(int position) {
    if(this->cursor != position) {
        this->cursor = position;
        if(position <= this->letters.size()) {
            QString letterID;
            if(position > 0)
                letterID = this->letters[position-1]->getLetterID();
            else letterID = this->letters[0]->getLetterID();
            /* Notify server */
            emit localCursorChangeNotify(position, letterID);
        }
    }
}

void FileHandler::localAlignChange(Qt::AlignmentFlag alignment, int cursorPosition, QString startID, QString lastID) {
    bool intervalStarted = false;
    /* Store alignment information for each letter locally */
    if(startID.compare("-1")!=0 && lastID.compare("-1")!=0) {
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
    }
    /* Send change to server */
    emit localAlignChangeNotify(alignment, cursorPosition, startID, lastID);
}

void FileHandler::remoteInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter, QTextCharFormat format, Qt::AlignmentFlag alignment,
                               bool modifiedLetter, QString modifiedLetterID, QJsonArray newposition) {

    // Get index and fractionals vector
    QVector<int> fractionals;


    if(!position.isEmpty()) {
        for(auto fractional : position) {
            fractionals.append(fractional.toInt());
        }

        QString letterID = QString::number(siteID).append("-").append(QString::number(siteCounter));
        Letter *newLetter = new Letter(newLetterValue, fractionals, letterID, format, alignment);

        // Ricerca binaria della posizione della nuova lettera nel vettore
        int index = this->letters.size();
        if(externalIndex > this->letters.size()/2) {
            int i = this->letters.size()-1;
            for(auto l=this->letters.crbegin(); l!=this->letters.crend(); l++) {
                bool comesFirst = this->letters[i--]->comesFirstRight(*newLetter, 0);
                if(!comesFirst)
                    index--;
                else break;
            }
        } else {
            int id = 0;
            for(Letter* l : this->letters) {
                bool comesFirst = l->comesFirstLeft(*newLetter, 0);
                id++;
                if(comesFirst){
                    index = id-1;
                    break;
                }
            }
        }
        this->letters.insert(this->letters.begin()+index, newLetter);
        /*Aggiornare la GUI*/
        emit readyRemoteInsert(newLetterValue, index, format, alignment, letterID);
    }
}

void FileHandler::collisionAlert(QString letterID, int newIndex, QVector<int> newPosition) {
    for(Letter* l : this->letters) {
        if(l->getLetterID().compare(letterID)==0) {
            l->setNewPosition(newPosition);
            break;
        }
    }
}

void FileHandler::remoteDelete(QString deletedLetterID) {
    int index = -1;

    for (int i=0; i<this->letters.size(); i++) {
        if(this->letters[i]->getLetterID().compare(deletedLetterID) == 0) {
            index = i;
            break;
        }
    }
    if(index >= 0) {
        this->letters.remove(index);
        /*Aggiornare la GUI*/
        emit readyRemoteDelete(index+1);
    }
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
    if(startID.compare("-1")!=0 && lastID.compare("-1")!=0) {
        for(Letter *l : this->letters) {
            if(!intervalStarted && l->getLetterID().compare(startID)==0) {
                intervalStarted = true;
                l->setAlignment(alignment);
                if(startID.compare(lastID)==0)
                    break;
            } else if(intervalStarted) {
                l->setAlignment(alignment);
                if(l->getLetterID().compare(lastID)==0)
                    break;
            }
        }
    }

    /* Update GUI */
    emit readyRemoteAlignChange(alignment, cursorPosition);
}

void FileHandler::remoteColorChange(QString startID, QString lastID, QColor color) {
    bool intervalStarted = false;
    int start = -1, end = -1, i = 0;
    for(Letter *l : this->letters) {
        if(!intervalStarted && l->getLetterID().compare(startID)==0) {
            intervalStarted = true;
            l->setColor(color);
            start = i;
            if(startID.compare(lastID)==0) {
                end = start;
                break;
            }
        } else if(intervalStarted) {
            l->setColor(color);
            if(l->getLetterID().compare(lastID)==0) {
                end = i;
                break;
            }
        }
        i++;
    }

    qDebug() << "start - end" << start << end;

    if(start!=-1 && end!=-1)
        emit readyRemoteColorChange(start, end, color);
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

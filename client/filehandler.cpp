#include "filehandler.h"
#include <QDebug>

FileHandler::FileHandler(QObject *parent)
  : QObject(parent), siteCounter(0)
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

void FileHandler::localInsert(int externalIndex, QChar newLetterValue, int clientID) {
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

    Letter *newLetter = new Letter(newLetterValue, position, letterID);
    qDebug() << "Letter inserted in position:" << position;
    this->letters.insert(this->letters.begin()+(externalIndex-1), newLetter);

    /*Inviare notifica via socket*/

    QJsonArray positionJsonArray;
    std::copy (position.begin(), position.end(), std::back_inserter(positionJsonArray));
    qDebug() << "Letter inserted in position:";
    emit localInsertNotify(newLetterValue, positionJsonArray, clientID, siteCounter, externalIndex);

}

void FileHandler::localDelete(int externalIndex) {
    qDebug() << "Removing letter at index " << externalIndex << "...";
    this->letters.remove(externalIndex);
    /*Inviare notifica via socket*/
    emit localDeleteNotify(externalIndex);
}

void FileHandler::remoteInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter) {
    // Get index and fractionals vector
    QVector<int> fractionals;

    if(!position.isEmpty()) {
        //int index = position.at(0).toInt();
        //position.removeAt(0);

        for(auto fractional : position) {
            fractionals.append(fractional.toInt());
        }

        QString letterID = QString::number(siteID).append("-").append(siteCounter);
        //Letter newLetter(newLetterValue, fractionals, letterID);

        this->letters.insert(this->letters.begin()+externalIndex, new Letter(newLetterValue, fractionals, letterID));
    }

    /*Aggiornare la GUI*/
    // parametri necessari = externalIndex + newLetterValue

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
    // parametri necessari: externalIndex = indice della lettera da rimuovere

}

void FileHandler::setValues(QVector<Letter *> letters, int fileid){
    if(!this->letters.empty()){
        this->letters.clear();
    }
    this->letters = letters;
    this->siteCounter=0;
    this->fileid = fileid;
}

QVector<Letter*> FileHandler::getVectorFile(){
    return this->letters;
}

int FileHandler::getFileId(){
    return this->fileid;
}

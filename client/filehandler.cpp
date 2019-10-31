#include "filehandler.h"
#include <QDebug>

FileHandler::FileHandler(int siteid, QObject *parent)
  : QObject(parent),
    siteCounter(siteid)
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
        Se gli indici esterni delle lettere a dx e sx sono diversi, il nuovo indice frazionario è (INT_MAX-ultimoIndiceLetteraPrecedente): se risultano uguali,
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
        position.insert(0, prevPos.at(0));

        int lastFractionalPrev, lastFractionalNext, newIndex;
        if(prevPos.size() > 1) {
            lastFractionalPrev = prevPos[prevPos.size()-1];
            if(prevPos.size() > 2) {
                for(int i=1; i<prevPos.size(); i++)
                    position.append(prevPos[i]);
            }
        } else lastFractionalPrev = 0;

        if(nextPos.size() > 1) {
            lastFractionalNext = nextPos[nextPos.size()-1];
        } else lastFractionalNext = 0;

        if(prevPos.at(0) < nextPos.at(0)) {
            newIndex = INT_MAX-lastFractionalPrev;
            if(newIndex == lastFractionalPrev || newIndex == INT_MAX) {
                position.append(lastFractionalPrev);
                position.append(INT_MAX/2);
            }
            else position.append(newIndex);
        } else {
            newIndex = (lastFractionalPrev + lastFractionalNext)/2;
            if(newIndex == 0 || newIndex == lastFractionalPrev) {
                position.append(lastFractionalPrev);
                position.append(INT_MAX/2);
            } else
                position.append(newIndex);
        }
    }
    return position;
}

void FileHandler::localInsert(int externalIndex, QChar newLetterValue, int clientID) {
    int lastIndex = 0;
    qDebug() << "Calcolo l'indice della lettera inserita localmente...";

    if(this->letters.size() > 0) {
        Letter lastLetter = this->letters.at(letters.size()-1);
        lastIndex = lastLetter.getIndex();
    }

    qDebug() << "Indice dell'ultima lettera del file: " << lastIndex;

    QVector<int> position, previousLetterPos, nextLetterPos;
    int internalIndex = -1;

    this->siteCounter++;

    QString letterID = QString::number(clientID).append("-").append(this->siteCounter);

    if(externalIndex >= this->letters.size()) {
        // la lettera inserita si trova alla fine del file
        if(this->letters.size() == 0) // caso prima lettera inserita
            internalIndex = 0;
        else
            internalIndex = lastIndex+1;
        position.insert(0, internalIndex);  //position = {internalIndex}
        qDebug() << "Lettera inserita alla fine del file in posizione " << internalIndex;
    } else {
        if(externalIndex > 0)   // lettera NON inserita all'inizio del file
            previousLetterPos = this->letters[externalIndex-1].getFractionalIndexes();
        nextLetterPos = this->letters[externalIndex].getFractionalIndexes();

        position = calculateInternalIndex(previousLetterPos, nextLetterPos);
        if(position.size() == 1 && position.at(0) == 0) {   // position = {0}
            //  Lettera inserita all'inizio del file: avrà indici {0,0}. Devo modificare la lettera che inizialmente aveva questi indici
            int last = nextLetterPos.size()-1;  // ultimo indice valido di nextLetterPos
            int value = this->letters[0].getFractionalIndexes()[last];  // prendo ultimo frazionario della lettera all'inizio del file
            if(value < INT_MAX) {
                this->letters[0].editIndex(last, value+1);
                // check che la seconda lettera non abbia gli stessi indici dopo la modifica
                if(this->letters.size() >= 2) {
                    if(this->letters[0].hasSameFractionals(this->letters[1])) {
                        this->letters[0].editIndex(last, value);
                        this->letters[0].addFractionalDigit(INT_MAX/2);
                    }
                }

            } else
                this->letters[0].addFractionalDigit(INT_MAX/2);
        }
    }

    Letter newLetter(newLetterValue, position, letterID);
    //this->letters.insert(externalIndex, newLetter);

    //insertLetterInArray(&newLetter);
    /*Inviare notifica via socket*/
    /*Cosa server al server?*/

    QJsonArray positionJsonArray;
    std::copy (position.begin(), position.end(), std::back_inserter(positionJsonArray));
    emit localInsertNotify(newLetterValue, positionJsonArray, clientID, siteCounter, externalIndex);
}

void FileHandler::localDelete(int externalIndex) {
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
        Letter newLetter(newLetterValue, fractionals, letterID);

        this->letters.insert(this->letters.begin()+externalIndex, newLetter);
    }

    /*Aggiornare la GUI*/

}

void FileHandler::remoteDelete(QString deletedLetterID) {
    int i = 0;

    for (Letter l : this->letters) {
        if(l.getLetterID().compare(deletedLetterID) == 0) {
            this->letters.remove(i);
            break;
        }
        i++;
    }

    /*Aggiornare la GUI*/

}

void FileHandler::setListFiles(QVector<QString> listFiles){
    this->listFiles = listFiles;
}

void FileHandler::setVectorLettersFile(QVector<Letter> lett){
    if(!this->letters.empty()){
        this->letters.clear();
    }
    this->letters = lett;
}

QVector<QString> FileHandler::getListFiles(){
    return this->listFiles;
}

QVector<Letter> FileHandler::getVectorFile(){
    return this->letters;
}

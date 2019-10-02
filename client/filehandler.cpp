#include "filehandler.h"
#include<bits/stdc++.h>

fileHandler::fileHandler()
{
    siteCounter = 0;
}

/**
  0. Incrementa contatore delle modifiche sul file (siteCounter)
  1. Se la lettera inserita si trova alla fine del file, l'indice interno corrisponderà al primo indice disponibile (lastIndex+1)
  2. Altrimenti, calcola l'indice interno intero e frazionario, accedendo al vettore di lettere del file
*/
void fileHandler::localInsert(int externalIndex, QChar newLetterValue, int clientID) {
    Letter lastLetter = this->letters.at(letters.size()-1);
    int lastIndex = lastLetter.getIndex();
    QVector<int> position, previousLetterPos, nextLetterPos;
    int internalIndex = -1;

    this->siteCounter++;

    QString letterID = QString::number(clientID).append("-").append(this->siteCounter);

    if(externalIndex >= this->letters.size()) {
        // la lettera inserita si trova alla fine del file
        internalIndex = lastIndex+1;
        position.insert(0, internalIndex);
    } else {
        if(externalIndex > 0)
            previousLetterPos = this->letters[externalIndex-1].getFractionalIndexes();
        nextLetterPos = this->letters[externalIndex].getFractionalIndexes();

        position = calculateInternalIndex(previousLetterPos, nextLetterPos);
        if(position.size() == 1 && position.at(0) == 0) {
            int last = nextLetterPos.size()-1;
            int value = this->letters[externalIndex].getFractionalIndexes()[last];
            if(value < INT_MAX)
                this->letters[externalIndex].editIndex(last, value+1);
            else
                this->letters[externalIndex].addFractionalDigit(INT_MAX/2);
        }
    }

    Letter newLetter(newLetterValue, position, letterID);
    this->letters.insert(externalIndex, newLetter);

    //insertLetterInArray(&newLetter);
    // invia messaggio a server
}

QVector<int> fileHandler::calculateInternalIndex(QVector<int> prevPos, QVector<int> nextPos) {
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
            newIndex = (lastFractionalPrev+INT_MAX)/2;
            if(newIndex == lastFractionalPrev) {
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

void fileHandler::localDelete(int externalIndex) {
    this->letters.remove(externalIndex);
    // invio messaggio al server
}

void fileHandler::remoteInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter) {
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
}

void fileHandler::remoteDelete(QString deletedLetterID) {
    int i = 0;

    for (Letter l : this->letters) {
        if(l.getLetterID().compare(deletedLetterID) == 0) {
            this->letters.remove(i);
            break;
        }
        i++;
    }
}


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


/*QVector<int> FileHandler::calculateInternalIndex(QVector<int> prevPos, QVector<int> nextPos) {
    QVector<int> position;

    // Set internal index
    if(prevPos.isEmpty()) { // externalIndex == 0 (insert inizio file)
        position.insert(0, 0);  // position.insert(0, nextPos.at(0)) ?
    } else {
        position.insert(0, prevPos.at(0));  // indice intero pari a quello della lettera che precede
        int lastFractionalPrev, lastFractionalNext, newIndex;

        if(prevPos.at(0) < nextPos.at(0)) { // indici interi diversi: non mi interessa il valore degli indici frazionari della lettera successiva, ma solo della precedente
            if(nextPos.size() > 1) {
                //position.clear();
                position.replace(0, nextPos.at(0));
                qDebug() << "Position calculated: " << position[0];
                return position;
            }

            if(prevPos.size() == 1) {
                newIndex = int(INT_MAX/2);
                position.append(newIndex);
            } else {
                bool inserted = false;
                int i = 1;
                while (!inserted && prevPos.size() > i) {
                    lastFractionalPrev = prevPos[i];    // media tra prima cifra frazionaria e il massimo INT_MAX: evito di portarmi dietro tutti i frazionari
                    newIndex = int(INT_MAX/2 + lastFractionalPrev/2);
                    if(newIndex <= lastFractionalPrev) {
                        if(newIndex+1 != INT_MAX) {
                            position.append(newIndex+1);
                            inserted = true;
                        } else {
                            position.append(lastFractionalPrev);
                            i++;
                        }
                    } else {
                        position.append(newIndex);
                        inserted = true;
                    }
                }
                if(!inserted)
                    position.append(int(INT_MAX/2));
            }
        } else {    // indici uguali
            if(prevPos.size() == nextPos.size()) {
                // Stesso numero di indici frazionari
                if(prevPos.size() > 1) {
                    lastFractionalPrev = prevPos[prevPos.size()-1];
                    lastFractionalNext = nextPos[nextPos.size()-1];
                } else {
                    lastFractionalPrev = 0;
                    lastFractionalNext = 0;
                }

                newIndex = int(lastFractionalNext/2 + lastFractionalPrev/2);

                if(newIndex == 0 || newIndex == lastFractionalPrev) {
                    // inserire ++ e gestirlo
                    if(lastFractionalPrev+1 == lastFractionalNext || lastFractionalPrev+1 == INT_MAX) {
                        qDebug() << "Doppia append";
                        position.append(lastFractionalPrev);
                        position.append(int(INT_MAX/2));
                    } else
                        position.append(lastFractionalPrev+1);
                } else
                    position.append(newIndex);

            } else {
                // Diverso numero di indici frazionari
                int n = (nextPos.size() > prevPos.size()) ? prevPos.size() : nextPos.size();
                bool foundDifferent = false;
                for(int i=1; i<n; i++) {    // copia gli indici frazionari uguali della lettera precedente
                    if(prevPos[i] == nextPos[i])
                        position.append(prevPos[i]);
                    else {
                        // appena trovo un indice diverso esco dal ciclo e salvo l'indice raggiunto
                        n = i;
                        foundDifferent = true;
                        break;
                    }
                }

                if(foundDifferent) {
                    lastFractionalPrev = prevPos[n];
                    lastFractionalNext = nextPos[n];
                } else {
                    if(prevPos.size() < nextPos.size()) {
                        lastFractionalPrev = 0;
                        lastFractionalNext = nextPos[n]; // primo indice subito dopo la fine di prevPos
                    } else {
                        lastFractionalPrev = prevPos[n];
                        lastFractionalNext = INT_MAX;
                    }
                }

                newIndex = int(lastFractionalNext/2 + lastFractionalPrev/2);

                if(newIndex > lastFractionalPrev) {
                    position.append(newIndex);
                } else {
                    if(lastFractionalPrev+1 < lastFractionalNext) {
                        position.append(lastFractionalPrev+1);
                    } else {
                        qDebug() << "Doppia append";
                        position.append(lastFractionalPrev);
                        if(n == prevPos.size())
                            position.append(int(INT_MAX/2));
                        else {
                            int id = int(prevPos[n]/2 + INT_MAX/2);
                            if(id == prevPos[n])
                                position.append(id+1);
                            else position.append(id);
                        }
                    }

                }
            }
        }
    }
    qDebug() << "Position calculated: ";
    for (auto p : position)
        qDebug() << p <<"-";

    return position;
}

void FileHandler::localInsert(int externalIndex, QChar newLetterValue, int clientID, QTextCharFormat format, Qt::AlignmentFlag alignment) {

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

    bool collision = false;

    if(externalIndex > this->letters.size()) {
        // la lettera inserita si trova alla fine del file
        if(this->letters.size() == 0) // caso prima lettera inserita
            internalIndex = 0;
        else
            internalIndex = lastIndex+1;
        position.insert(0, internalIndex);  //position = {internalIndex}
        qDebug() << "Lettera inserita alla fine del file in posizione " << internalIndex;
    } else {
        if(externalIndex > 1)   // lettera NON inserita all'inizio del file CIAO -> CIAGO
            previousLetterPos = this->letters[externalIndex-2]->getFractionalIndexes();
        nextLetterPos = this->letters[externalIndex-1]->getFractionalIndexes();

        position = calculateInternalIndex(previousLetterPos, nextLetterPos);

        if(position.size() == 1 && position.at(0) == 0) {   // Inizio file [position = [0]]
            //  Lettera inserita all'inizio del file: avrà indici {0}. Devo modificare la lettera che inizialmente aveva questi indici
            if(nextLetterPos.size() == 1 && nextLetterPos[0] == 0) {    // COLLISIONE: tutte e due le lettere hanno indici [0]
                // se ci sono gia due lettere, modifico l'indice in base a quello della successiva, altrimenti lo incremento di 1 semplicemente
                if(this->letters.size() >= 2) {
                    QVector<int> secondLetterIndexes = this->letters[1]->getFractionalIndexes();
                    if(secondLetterIndexes[0] == 0) {
                        // calcolo indice a metà tra lettera 1 (indice [0]) e lettera 2 (indice [0,x,..])
                        if(secondLetterIndexes.size() > 1) {    // dovrebbe sempre entrare qui (l'indice [0] era già assegnato all'altra lettera)
                            int firstFractional = secondLetterIndexes[1];
                            int newIndex;
                            if(firstFractional > INT_MAX/2)
                                newIndex = INT_MAX/2;   // -> (1) [0, INT_MAX/2], (2) [0,firstFractional,...]
                            else {
                                newIndex = int(firstFractional/2);
                                if(newIndex == 0) {
                                    if(firstFractional == 1) {  // -> (1) [0,0,INT_MAX/2], (2) [0,1,..]
                                        this->letters[0]->addFractionalDigit(0);
                                        newIndex = INT_MAX/2;
                                    } else
                                        newIndex = 1;   // -> (1) [0,1], (2) [0,firstFractional,...]
                                }
                            }
                            this->letters[0]->addFractionalDigit(newIndex);
                        }

                    } else if(secondLetterIndexes[0] == 1 && secondLetterIndexes.size() == 1) {
                            this->letters[0]->addFractionalDigit(INT_MAX/2);    // -> [0, INT_MAX/2]
                    } else
                        this->letters[0]->editIndex(0, 1);  // La seconda lettera ha indici [1,x,..] o l'indice inziale della seconda lettera è > 1 => [1] è disponibile
                } else
                    this->letters[0]->editIndex(0, 1);  // [0] della lettera gia presente diventa [1]
                qDebug() << "COLLISIONE INIZIO FILE";
                collision = true;
            }
        }
    }

    Letter *newLetter = new Letter(newLetterValue, position, letterID, format, alignment);

    qDebug() << "Letter inserted in position:" << position << " (external index " << externalIndex <<")";
    this->letters.insert(this->letters.begin()+(externalIndex-1), newLetter);

    /*Inviare notifica via socket

    if(!collision)
        emit localInsertNotify(newLetter, clientID, siteCounter, externalIndex, false, nullptr);
    else emit localInsertNotify(newLetter, clientID, siteCounter, externalIndex, true, this->letters[1]);
    //emit localInsertNotify(newLetterValue, positionJsonArray, clientID, siteCounter, externalIndex, format, alignment);
}*/

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
        //letterIDs->append(letterID);
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
            //qDebug()<<"FONT STRING" << fontString;
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
    /*QVector<int> fractionals, newpos;

    if(modifiedLetter) {
        for(auto pos: newposition)
            newpos.append(pos.toInt());
        for(Letter* l : this->letters) {
            if(l->getLetterID().compare(modifiedLetterID)==0) {
                l->setNewPosition(newpos);
                break;
            }
        }
    }

    if(!position.isEmpty()) {
        //int index = position.at(0).toInt();
        //position.removeAt(0);

        for(auto fractional : position) {
            fractionals.append(fractional.toInt());
        }

        QString letterID = QString::number(siteID).append("-").append(QString::number(siteCounter));
        Letter *newLetter = new Letter(newLetterValue, fractionals, letterID, format, alignment);

        int index=0;
        bool inserted = false;
        for(Letter* l : this->letters) {
            if(l->comesFirst(*newLetter))
                index++;
            else {
                this->letters.insert(this->letters.begin()+index, newLetter);
                inserted = true;
                break;
            }
        }
        if(!inserted)
            this->letters.insert(this->letters.begin()+index, newLetter);*/
        /*Aggiornare la GUI*/
        emit readyRemoteInsert(newLetterValue, index, format, alignment, letterID);
        //this->letters.insert(this->letters.begin()+externalIndex-1, new Letter(newLetterValue, fractionals, letterID, format, alignment));
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

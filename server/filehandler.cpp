#include "filehandler.h"

FileHandler::FileHandler(const QVector<Letter*>&& lett, int fileid, QObject *parent) : QObject(parent) {
    this->letters = lett;
    id = fileid;
}

void FileHandler::insertActiveUser(QTcpSocket *user){
    users.append(user);
    counter_user++;
}

void FileHandler::removeActiveUser(QTcpSocket *user){
    users.removeOne(user);
    counter_user--;
    if(counter_user == 0){
        // Salvarlo in memoria secondaria, io lo farei con un segnale
        QJsonObject object;
        QJsonArray array;
        for(Letter* lett: letters){
           array.append(lett->toJSon());
        }
        object.insert("letterArray",array);

        QFile file(QString::number(id));
        file.open(QFile::WriteOnly|QFile::Truncate); // this mode clear the content of a file

        if ( file.open(QFile::WriteOnly|QFile::Truncate) )
        {
            QTextStream stream( &file );
            stream << QJsonDocument(object).toJson() << endl;
        }
    }
}

/**
 * Metodo che gestisce inserimento remoto di una lettera:
 * - ricava indice INTERNO della lettera inserita
 * - ricava gli indici frazionali
 * - se il vettore di lettere del file è vuoto nella posizione indicata dall'indice, la lettera viene inserita in append;
 * - altrimenti, vi è uno shift a destra delle lettere con inserimento nella posizione corretta.
 * Casi particolari:
 * - inserimento equivalente da parte di utenti diversi.
 * */
void FileHandler::remoteInsert(QJsonArray position, QChar newLetterValue, int externalIndex,
                               int siteID, int siteCounter, QByteArray message, QTcpSocket *client) {
    // Get index and fractionals vector
    QVector<int> fractionals;


    if(!position.isEmpty()) {
        //int index = position.at(0).toInt();
        //position.removeAt(0);

        for(auto fractional : position) {
            fractionals.append(fractional.toInt());
        }

        QString letterID = QString::number(siteID).append("-").append(siteCounter);
        Letter *newLetter = new Letter(newLetterValue, fractionals, letterID);

        if(externalIndex < this->letters.size()) {
            if(newLetter->hasSameFractionals(*(this->letters[externalIndex]))) {
                if(newLetterValue == this->letters[externalIndex]->getLetterValue())
                    // stessa lettera inserita nella stessa posizione da utenti diversi => ignora insert
                    return;
                else if (siteID > this->letters[externalIndex]->getSiteID()) {  // lettera diversa inserita nella stessa posizione => inserimento in ordine di siteID
                    this->letters.insert(this->letters.begin()+externalIndex+1, newLetter);
                    // propaga informazione con indice modificato
                    emit remoteInsertNotify(this->users, message, true, externalIndex+1);
                    return;
                }
            }
        }

        this->letters.insert(this->letters.begin()+externalIndex-1, newLetter);

        /*
        Letter lastLetter = this->letters.at(letters.size()-1);
        int lastIndex = lastLetter.getIndex();

        if(index > lastIndex) {
            // la lettera inserita si trova alla fine del file
            this->letters.append(*newLetter);
        } else {
            // lettera inserita all'interno del file
            bool inserted = false;

            // Ricerca posizione in cui inserire la nuova lettera nel vettore
            for(int i=0; i<this->letters.size() && !inserted; i++) {
                if(newLetter->comesFirst(this->letters[i])) {
                    letters.insert(this->letters.begin()+i, *newLetter);
                    inserted = true;
                } else if(newLetter->hasSameFractionals(this->letters[i])) {
                    if(newLetterValue == this->letters[i].getLetterValue())
                        // stessa lettera inserita nella stessa posizione da utenti diversi => ignora insert
                        return;
                    else if (siteID <= this->letters[i].getSiteID()) {  // lettera diversa inserita nella stessa posizione => inserimento in ordine di siteID
                        this->letters.insert(this->letters.begin()+i, *newLetter);
                        inserted = true;
                    } else if (siteID > this->letters[i].getSiteID()) {
                        this->letters.insert(this->letters.begin()+i+1, *newLetter);
                        inserted = true;
                    }
                }
            }
        }*/

        // Notifica gli altri client inviando lo stesso messaggio
        emit remoteInsertNotify(this->users, message, false, 0, client);
    }
}

void FileHandler::remoteDelete(QString deletedLetterID,  QByteArray message) {
    int i = 0;

    for (Letter *l : this->letters) {
        if(l->getLetterID().compare(deletedLetterID) == 0) {
            this->letters.remove(i);
            break;
        }
        i++;
    }
    // Notifica gli altri client inviando lo stesso messaggio
    emit remoteDeleteNotify(this->users, message);
}

QVector<QTcpSocket*> FileHandler::getUsers(){
    return this->users;
}

QVector<Letter*> FileHandler::getLetter(){
    return this->letters;
}


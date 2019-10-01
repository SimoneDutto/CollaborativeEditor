#include "filehandler.h"

FileHandler::FileHandler()
{

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
    }
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
    // Notifica gli altri client inviando lo stesso messaggio
}

#include "filehandler.h"
#include <QDebug>

FileHandler::FileHandler(const QVector<Letter*>&& lett, int fileid, QObject *parent) : QObject(parent) {
    this->letters = lett;
    id = fileid;
}

void FileHandler::insertActiveUser(QTcpSocket* user, int siteCounter, int siteId){
    users.append(user);
    counter_user++;
    usersSiteCounters.insert(user, siteCounter);
    QJsonObject json;
    QByteArray sendSize;

    json.insert("type", "USER_CONNECT");
    json.insert("siteId", siteId);
    for(QTcpSocket* u: users){
        if(u == user) continue;
        if(u->state() == QAbstractSocket::ConnectedState){
            qint32 msg_size = QJsonDocument(json).toJson().size();
            QByteArray toSend;
            u->write(toSend.number(msg_size), sizeof (long int));
            u->waitForBytesWritten();
            if(u->write(QJsonDocument(json).toJson()) == -1){
                qDebug() << "File info failed to send";
                return;
            } //write the data itself
            u->waitForBytesWritten();
        }
    }
}

void FileHandler::removeActiveUser(QTcpSocket *user, int siteId){
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

        if ( file.open(QFile::WriteOnly|QFile::Truncate) )
        {
            QTextStream stream( &file );
            stream << QJsonDocument(object).toJson() << endl;
        }
        file.close();
    }

    QJsonObject json;
    QByteArray sendSize;

    json.insert("type", "USER_DISCONNECT");
    json.insert("siteId", siteId);
    for(QTcpSocket* u: users){
        if(u == user) continue;
        if(u->state() == QAbstractSocket::ConnectedState){
            qint32 msg_size = QJsonDocument(json).toJson().size();
            QByteArray toSend;
            u->write(toSend.number(msg_size), sizeof (long int));
            u->waitForBytesWritten();
            if(u->write(QJsonDocument(json).toJson()) == -1){
                qDebug() << "File info failed to send";
                return;
            } //write the data itself
            u->waitForBytesWritten();
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
                               int siteID, int siteCounter, QByteArray message, QTcpSocket *client, QTextCharFormat format) {
    // Get index and fractionals vector
    QVector<int> fractionals;


    if(!position.isEmpty()) {
        //int index = position.at(0).toInt();
        //position.removeAt(0);

        if(this->usersSiteCounters.contains(client)) {
            QMap<QTcpSocket*, int>::iterator i = this->usersSiteCounters.find(client);
            i.value() = siteCounter;
            qDebug() << "Site counter updated after insert = " << siteCounter;
        }

        for(auto fractional : position) {
            fractionals.append(fractional.toInt());
        }

        QString letterID = QString::number(siteID).append("-").append(QString::number(siteCounter));
        Letter *newLetter = new Letter(newLetterValue, fractionals, letterID, format);
        //newLetter->setStyle(style);

        if(externalIndex < this->letters.size()) {
            if(newLetter->hasSameFractionals(*(this->letters[externalIndex]))) {
                if(newLetterValue == this->letters[externalIndex]->getLetterValue())
                    // stessa lettera inserita nella stessa posizione da utenti diversi => ignora insert
                    return;
                else if (siteID > this->letters[externalIndex]->getSiteID()) {  // lettera diversa inserita nella stessa posizione => inserimento in ordine di siteID
                    this->letters.insert(this->letters.begin()+externalIndex+1, newLetter);
                    // propaga informazione con indice modificato
                    emit remoteInsertNotify(this->users, message, true, externalIndex+1, client);
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

void FileHandler::remoteDelete(QString deletedLetterID,  QByteArray message, QTcpSocket* client, int siteCounter) {
    int i = 0;

    if(this->usersSiteCounters.contains(client)) {
        QMap<QTcpSocket*, int>::iterator i = this->usersSiteCounters.find(client);
        i.value() = siteCounter;
        qDebug() << "Site counter updated after delete = " << siteCounter;
    }

    for (Letter *l : this->letters) {
        if(l->getLetterID().compare(deletedLetterID) == 0) {
            this->letters.remove(i);
            break;
        }
        i++;
    }
    // Notifica gli altri client inviando lo stesso messaggio
    emit remoteDeleteNotify(this->users, message, client);
}

void FileHandler::changeStyle(QString initialIndex, QString lastIndex, QString format, QTcpSocket *client, QByteArray message) {
    bool intervalStarted = false;

    for(Letter *l : this->letters) {
        if(l->getLetterID().compare(initialIndex) == 0 || intervalStarted) {
            intervalStarted = true;
            l->setStyleFromString(format);
            if(l->getLetterID().compare(lastIndex) == 0)
                break;
        }
    }

    /* Propagate change to other clients working on the same file */
    emit remoteStyleChangeNotify(this->users, message, client);
}

QVector<QTcpSocket*> FileHandler::getUsers(){
    return this->users;
}

QVector<Letter*> FileHandler::getLetter(){
    return this->letters;
}

int FileHandler::getSiteCounter(QTcpSocket *user) {
    return this->usersSiteCounters.value(user);
}

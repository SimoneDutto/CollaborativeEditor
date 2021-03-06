﻿#include "filehandler.h"
#include <QDebug>

FileHandler::FileHandler(const QVector<Letter*>&& lett, int fileid, QObject *parent) : QObject(parent) {
    this->letters = lett;
    id = fileid;
}

void FileHandler::insertActiveUser(QTcpSocket* user, int siteCounter, QString username, int userID, int cursorPosition){
    users.append(user);
    counter_user++;
    usersSiteCounters.insert(user, siteCounter);
    usersCursorPosition.insert(user, cursorPosition);
    QJsonObject json;
    QByteArray sendSize;

    json.insert("type", "USER_CONNECT");
    json.insert("username", username);
    json.insert("userID", userID);
    json.insert("cursor", cursorPosition);
    for(QTcpSocket* u: users){
        if(u == user) continue;
        if(u->state() == QAbstractSocket::ConnectedState){
            qDebug() << "Invio ";
            qint32 msg_size = QJsonDocument(json).toJson().size();
            QByteArray toSend;
            u->write(toSend.number(msg_size), sizeof (quint64));
            u->waitForBytesWritten();
            if(u->write(QJsonDocument(json).toJson()) == -1){
                qDebug() << "File info failed to send";
                return;
            } //write the data itself
            u->waitForBytesWritten();
        }
    }
}

void FileHandler::removeActiveUser(QTcpSocket *user, QString username, int userID){
    users.removeOne(user);
    usersSiteCounters.remove(user);
    usersCursorPosition.remove(user);
    counter_user--;

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

    QJsonObject json;
    QByteArray sendSize;

    json.insert("type", "USER_DISCONNECT");
    json.insert("username", username);
    json.insert("userID", userID);
    for(QTcpSocket* u: users){
        if(u == user) continue;
        if(u->state() == QAbstractSocket::ConnectedState){
            qDebug() << "Invio ";
            qint32 msg_size = QJsonDocument(json).toJson().size();
            QByteArray toSend;
            u->write(toSend.number(msg_size), sizeof (quint64));
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
                               int siteID, int siteCounter, QByteArray message, QTcpSocket *client, QTextCharFormat format, Qt::AlignmentFlag alignment,
                               bool modifiedLetter, QString modifiedID, QJsonArray newposition) {
    // Get index and fractionals vector
    QVector<int> fractionals;


    if(!position.isEmpty() && this->usersSiteCounters.contains(client)) {
        QMap<QTcpSocket*, int>::iterator i = this->usersSiteCounters.find(client);
        i.value() = siteCounter;

        for(auto fractional : position) {
            fractionals.append(fractional.toInt());
        }

        QString letterID = QString::number(siteID).append("-").append(QString::number(siteCounter));
        Letter *newLetter = new Letter(newLetterValue, fractionals, letterID, format, alignment);

        // Ricerca binaria della posizione della nuova lettera nel vettore
        int index = this->letters.size();
        if(externalIndex > static_cast<int>(this->letters.size()/2)) {
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
        for (Letter *l : this->letters) qDebug() << l->getLetterValue();
        emit remoteInsertNotify(this->users, message, false, index, fractionals, client);
    }
}

QVector<int> FileHandler::modifyPositionIndexes(QVector<int> currentIndexes, int externalIndex) {
    int lastIndex = currentIndexes[currentIndexes.size()-1];
    lastIndex++;
    currentIndexes[currentIndexes.size()-1] = lastIndex;
    if(externalIndex == this->letters.size() || !(this->letters[externalIndex]->getFractionalIndexes() == currentIndexes)) {
        // lettera a fine file (non devo fare confronto con lettere successive) oppure non c'è collisione
        return currentIndexes;
    } else {
        // collisione con lettera successiva: ripristino last index + aggiungo indice frazionario
        currentIndexes[currentIndexes.size()-1] = lastIndex - 1;
        currentIndexes.append(int(INT_MAX/2));
    }
}

void FileHandler::remoteDelete(QString deletedLetterID,  QByteArray message, QTcpSocket* client, int siteCounter) {
    int index = 0;

    if(this->usersSiteCounters.contains(client)) {
        QMap<QTcpSocket*, int>::iterator i = this->usersSiteCounters.find(client);
        i.value() = siteCounter;
        qDebug() << "Site counter updated after delete = " << siteCounter;
        for (int i=0; i<this->letters.size(); i++) {
            if(this->letters[i]->getLetterID().compare(deletedLetterID) == 0) {
                index = i;
                break;
            }
        }
        if(index >= 0) {
            this->letters.remove(index);
            // Notifica gli altri client inviando lo stesso messaggio
            emit remoteDeleteNotify(this->users, message, client);
        }
    }
}

void FileHandler::changeStyle(QString initialIndex, QString lastIndex, QString format, QString font, QTcpSocket *client, QByteArray message) {
    bool intervalStarted = false;

    for(Letter *l : this->letters) {
        if(l->getLetterID().compare(initialIndex) == 0 || intervalStarted) {
            intervalStarted = true;
            l->setStyleFromString(format, font);
            if(l->getLetterID().compare(lastIndex) == 0)
                break;
        }
    }

    /* Propagate change to other clients working on the same file */
    emit remoteStyleChangeNotify(this->users, message, client);
}

void FileHandler::changeAlign(Qt::AlignmentFlag align, QString startID, QString lastID, QTcpSocket *client, QByteArray message) {
    bool intervalStarted = false;
    /* Store alignment information for each letter locally */
    for(Letter *l : this->letters) {
        if(!intervalStarted && l->getLetterID().compare(startID)==0) {
            intervalStarted = true;
            l->setAlignment(align);
            if(startID.compare(lastID)==0)
                break;
        } else if(intervalStarted) {
            l->setAlignment(align);
            if(l->getLetterID().compare(lastID)==0)
                break;
        }
    }

    /* Propagate change to other clients working on the same file */
    emit remoteAlignChangeNotify(this->users, message, client);
}

void FileHandler::changeCursor(QTcpSocket *client, QByteArray message, int position) {
    if(usersCursorPosition[client] != position) {
        usersCursorPosition[client] = position;
        emit remoteCursorChangeNotify(this->users, message, client);
    }
}

void FileHandler::changeColor(QString startID, QString lastID, QString colorName, QTcpSocket *client, QByteArray message) {
    bool intervalStarted = false;
    QColor color(colorName);
    /* Store alignment information for each letter locally */
    for(Letter *l : this->letters) {
        if(!intervalStarted && l->getLetterID().compare(startID)==0) {
            intervalStarted = true;
            l->setColor(color);
            if(startID.compare(lastID)==0)
                break;
        } else if(intervalStarted) {
            l->setColor(color);
            if(l->getLetterID().compare(lastID)==0)
                break;
        }
    }

    emit remoteColorChangeNotify(this->users, message, client);
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

int FileHandler::getUserCursorPosition(QTcpSocket *user) {
    return this->usersCursorPosition.value(user);
}

bool FileHandler::thereAreUsers() {
    return !users.isEmpty();
}

#include "filesystem.h"
#include <QCryptographicHash>
#include <QtEndian>

#define STR_SALT_KEY "qwerty"
#define SALT_FILE "2410"
#define DATA_SIZE 1024

// class to handle all files in a map

FileSystem *FileSystem::instance = nullptr;

FileSystem* FileSystem::getInstance(){
    if(!instance){
        instance = new FileSystem();
        const QString DRIVER("QSQLITE");
        if(QSqlDatabase::isDriverAvailable(DRIVER)){
            instance->db = QSqlDatabase::addDatabase(DRIVER);
            instance->db.setDatabaseName("user.db");
            if (!instance->db.open())
            {
                 qDebug() << "Error: connection with database fail";
            }
            else
            {
                qDebug() << "Database: connection ok";
            }

        }
    }
    return FileSystem::instance;
}

void FileSystem::createFile(QString filename, QTcpSocket *socket){
    FileHandler *fh;
    auto id = sock_id.find(socket);
    if(id == sock_id.end()) return;//il socket è autenticato o no

    auto file = sock_file.find(socket);
    if(file != sock_file.end()){
        // disconnessione di un client da un file
        FileHandler *fh = files.at(file->second);
        fh->removeActiveUser(socket, sock_username.at(socket));
    }
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM files WHERE userid=((:userid) AND filename=(:filename))");
    query.bindValue(":filename", filename);
    query.bindValue(":userid", id->second);
    int count = -1;
    if (query.exec())
    {
        if (query.next())
        {
            count = query.value(0).toInt();
        }
    }
    else{
        qDebug() << "Query not executed";
    }
    if(count != 0){
        qDebug("This filename is already taken by the user");
    }
    query.prepare("SELECT COUNT(fileid) FROM files");
    int fileid=0;
    if (query.exec())
    {
        if (query.next())
        {
            fileid = query.value(0).toInt();
        }
    }
    else{
        qDebug() << "Query not executed";
    }

    query.prepare("INSERT INTO files(Userid,Filename,FileId,SiteCounter) VALUES ((:userid), (:filename),(:fileid),0)");
    query.bindValue(":filename", filename);
    query.bindValue(":userid", id->second);
    query.bindValue(":fileid", fileid);
    if (query.exec()){
        QFile m_file (QString::number(fileid)); // crea il file col nome id
        m_file.open(QFile::ReadOnly);

        QVector<Letter*> letters;

        fh = new FileHandler(std::move(letters), fileid);
        fh->insertActiveUser(socket,0, sock_username.at(socket));

        sock_file.insert(std::pair<QTcpSocket*, int> (socket, fileid));
        //sock_file.insert(socket, fileid); //associate file to socket
        connect(fh, SIGNAL(remoteInsertNotify(QVector<QTcpSocket*>, QByteArray, bool, int, QTcpSocket*)),
                this, SLOT(sendInsert(QVector<QTcpSocket*>, QByteArray, bool, int, QTcpSocket*)));

        connect(fh, SIGNAL(remoteDeleteNotify(QVector<QTcpSocket*>, QByteArray, QTcpSocket*)),
                this, SLOT(sendDelete(QVector<QTcpSocket*>, QByteArray, QTcpSocket*)));

        connect(fh, SIGNAL(remoteStyleChangeNotify(QVector<QTcpSocket*>, QByteArray, QTcpSocket*)),
                this, SLOT(sendStyleChange(QVector<QTcpSocket*>, QByteArray, QTcpSocket*)));

        files.insert(std::pair<int, FileHandler*> (fileid, fh));
        QJsonObject file_info;
        file_info.insert("type", "NEW");
        file_info.insert("fileid", fileid);

        // Send size of message "OPEN"
        if(socket->state() == QAbstractSocket::ConnectedState) {
            qDebug() << "Invio file";
            QByteArray qarray = QJsonDocument(file_info).toJson();
            qint32 msg_size = qarray.size();
            QByteArray toSend;
            socket->write(toSend.number(msg_size), sizeof (long int));
            socket->waitForBytesWritten();
            if(socket->write(qarray) == -1){
                qDebug() << "File info failed to send";
                return;
            } //write the data itself
            socket->waitForBytesWritten();
        }

        qDebug() << "Insert executed";
        return;
    }
    else{
        qDebug() << "Insert not executed";
    }

    return;

}

// Da testare
void FileSystem::accessFile(QString URI, QTcpSocket *socket){
    auto socket_id = sock_id.find(socket);

    if(socket_id == sock_id.end()){
        qDebug() << "Socket non autenticato";
        return;//il socket è autenticato o no
    }
    // decritta l'URI e diventa FileID
    QByteArray ba;
    ba.append(URI);
    int fileid = QByteArray::fromBase64(ba).toInt();
    qDebug() << fileid;
    QSqlQuery query;
    QString filename;
    query.prepare("SELECT DISTINCT Filename FROM FILES WHERE FileId=(:fileid)");
    query.bindValue(":fileid", fileid);
    if (query.exec()) {
        if (query.next())
        {
            filename = query.value(0).toString();
            qDebug() << filename;
        }
        else{
            filename = "nofile";
        }
    } else {
        qDebug() << "Error! Filename not retrieved.";
        return;
    }

    int siteCounter=1;
    if (!(filename.compare("nofile")==0)){
        query.prepare("INSERT INTO Files(Filename, FileId, UserId, SiteCounter) VALUES((:filename), (:fileid), (:userid), (:siteCounter))");
        query.bindValue(":filename", filename);
        query.bindValue(":fileid", fileid);
        query.bindValue(":userid", socket_id->second);
        query.bindValue(":siteCounter", siteCounter); // chiedere a Deb 0

        if (query.exec()){
            qDebug() << "inserted";
            QJsonObject json;
            json.insert("type", "ACCESS_RESPONSE");
            json.insert("filename", filename);
            json.insert("fileid", fileid);
            sendJson(json, socket);
            return;
        }
        else{
            qDebug() << "Query not executed";
        }
    }
    QJsonObject json;
    json.insert("type", "ACCESS_RESPONSE");
    json.insert("filename", 0);
    json.insert("fileid", -1);
    sendJson(json, socket);

}
void FileSystem::sendFile(int fileid, QTcpSocket *socket){
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    QFile inFile(QString::number(fileid));
    inFile.open(QFile::ReadOnly);

    //TODO: controllare che il client ha accesso

    auto socket_id = sock_id.find(socket);

    if(socket_id == sock_id.end()) return;//il socket è autenticato o no
    auto file = sock_file.find(socket);

    if(file != sock_file.end()){
        // disconnessione di un client da un file
        FileHandler *fh = files.at(fileid);
        fh->removeActiveUser(socket, sock_username.at(socket));
    }
    QSqlQuery query;
    int siteCounter=0;
    query.prepare("SELECT siteCounter FROM FILES WHERE FileId=(:fileid) AND UserId=(:userid)");
    query.bindValue(":fileid", fileid);
    query.bindValue(":userid", socket_id->second);
    if (query.exec()) {
        if (query.next())
        {
            siteCounter = query.value(0).toInt();
        }
    } else {
        qDebug() << "Error! SiteCounter not retrieved.";
        return;
    }
    QByteArray ba;
    ba.append(QString::number(fileid));
    QString URI = ba.toBase64();

    auto it = files.find(fileid);
    if (it != files.end()){
        // il file è già in memoria principale e può essere mandato
        // serializzarlo

        int size = static_cast<int>(inFile.size());
        QJsonObject file_info;
        file_info.insert("type", "OPEN");
        file_info.insert("fileid", fileid);
        file_info.insert("size", size);
        file_info.insert("siteCounter", siteCounter);
        file_info.insert("URI", URI);
        qDebug() << "Ciao";
        QJsonArray userArray;
        QVector<QTcpSocket*> users = it->second->getUsers();
        for(QTcpSocket* user: users){
            userArray.push_back(sock_username.at(user));
        }
        file_info.insert("activeUser", userArray);

        // Send size of message "OPEN"
        if(socket->state() == QAbstractSocket::ConnectedState) {
            qDebug() << "Invio file";
            QByteArray qarray = QJsonDocument(file_info).toJson();
            qint32 msg_size = qarray.size();
            QByteArray toSend;
            socket->write(toSend.number(msg_size), sizeof (long int));
            socket->waitForBytesWritten();
            if(socket->write(qarray) == -1){
                qDebug() << "File info failed to send";
                return;
            } //write the data itself
            socket->waitForBytesWritten();
        }

        QJsonArray file_array;
        for(Letter* lett: it->second->getLetter()){
           file_array.append(lett->toJSon());
        }
        QJsonObject obj;
        obj.insert("letterArray", file_array);
        QByteArray splitToSend = QJsonDocument(obj).toJson();
        int from = 0, chunk;
        int remaining = splitToSend.size();

        while(remaining > 0){
            if(remaining > DATA_SIZE)
                chunk = DATA_SIZE;
            else
                chunk = remaining;
            //QByteArray qa = inFile.read(chunk);
            qDebug() << "emitting dataRead() da file serializzato";
            remaining -= chunk;
            qDebug() << "--------------------------------------------------";
            qDebug() << splitToSend.mid(from, chunk).data();
            qDebug() << "--------------------------------------------------";
            if(remaining > 0)
                emit dataRead(splitToSend.mid(from, chunk), socket, remaining);
            else if (remaining == 0)
                emit dataRead(splitToSend.mid(from, chunk+1), socket, remaining);
            from += chunk;
        }

        qDebug() << "File sent";
        FileHandler *fh = it->second;
        fh->insertActiveUser(socket, siteCounter, sock_username.at(socket));

        sock_file.insert(std::pair<QTcpSocket*, int> (socket, fileid)); //associate file to socket

        return;
    }
    else {
        qDebug() << "Inizio l'invio del file";
        // apre il file, lo scrive in un DataStream che poi invierà
        QByteArray buffer_tot;
        int size = static_cast<int>(inFile.size());
        qDebug() << size;

        QJsonObject file_info;
        file_info.insert("type", "OPEN");
        file_info.insert("fileid", fileid);
        file_info.insert("size", size);
        file_info.insert("siteCounter", siteCounter);
        file_info.insert("URI", URI);



        int remaining = size;

        //manda il file info
        if(socket->state() == QAbstractSocket::ConnectedState){
            qDebug() << "Invio file_info";
            qint32 msg_size = QJsonDocument(file_info).toJson().size();
            QByteArray toSend;
            socket->write(toSend.number(msg_size), sizeof (long int));
            socket->waitForBytesWritten();
            if(socket->write(QJsonDocument(file_info).toJson()) == -1){
                qDebug() << "File info failed to send";
                return;
            } //write the data itself
            socket->waitForBytesWritten();
        }
        // manda i chunk
        while(remaining > 0)
        {
            int chunk;
            if(remaining > DATA_SIZE)
                chunk = DATA_SIZE;
            else
                chunk = remaining;
            QByteArray qa = inFile.read(chunk);
            remaining -= chunk;
            buffer_tot.append(qa);
            qDebug() << "emitting dataRead(), remaining = " << remaining << "chunk = " << chunk;
            emit dataRead(qa, socket, remaining);
        }
        inFile.close();

        qDebug() << "File sent";

        // lo salva in memoria ram
        QJsonDocument document = QJsonDocument::fromJson(buffer_tot);
        QJsonObject object = document.object();
        QJsonValue value = object.value("letterArray");
        QJsonArray letterArray = value.toArray();

        QVector<Letter*> letters;

        foreach (const QJsonValue& v, letterArray)
        {
            QChar letter = v.toObject().value("letter").toString().at(0);
            QString ID = v.toObject().value("letterID").toString();
            QJsonArray array_tmp = v.toObject().value("position").toArray();
            QVector<int> fractionals;
            for(auto fractional : array_tmp) {
                fractionals.append(fractional.toInt());
            }
            // GET FORMAT LETTER
            QTextCharFormat format;
            bool isBold = v.toObject().value("isBold").toBool();
            bool isItalic = v.toObject().value("isItalic").toBool();
            bool isUnderlined = v.toObject().value("isUnderlined").toBool();

            if(isBold)
                format.setFontWeight(75);
            else format.setFontWeight(50);
            if(isItalic)
                format.setFontItalic(true);
            else format.setFontItalic(false);
            if(isUnderlined)
                format.setFontUnderline(true);
            else format.setFontUnderline(false);

            Letter *letter_tmp = new Letter(letter, fractionals, ID, format);
            letters.append(std::move(letter_tmp));
        }
        FileHandler *fh = new FileHandler(std::move(letters), fileid);
        connect(fh, SIGNAL(remoteInsertNotify(QVector<QTcpSocket*>, QByteArray, bool, int, QTcpSocket*)),
                this, SLOT(sendInsert(QVector<QTcpSocket*>, QByteArray, bool, int, QTcpSocket*)));

        connect(fh, SIGNAL(remoteDeleteNotify(QVector<QTcpSocket*>, QByteArray, QTcpSocket*)),
                this, SLOT(sendDelete(QVector<QTcpSocket*>, QByteArray, QTcpSocket*)));

        connect(fh, SIGNAL(remoteStyleChangeNotify(QVector<QTcpSocket*>, QByteArray, QTcpSocket*)),
                this, SLOT(sendStyleChange(QVector<QTcpSocket*>, QByteArray, QTcpSocket*)));

        fh->insertActiveUser(socket, siteCounter, sock_username.at(socket));

        files.insert(std::pair<int, FileHandler*> (fileid, fh));
        sock_file.insert(std::pair<QTcpSocket*, int> (socket, fileid)); //associate file to socket
        qDebug() << "File saved in RAM";
    }
    // Send notification the file is opened by the user

}

/*QByteArray FileSystem::IntToArray(qint32 source) //Use qint32 to ensure that the number have 4 bytes
{
    //Avoid use of cast, this is the Qt way to serialize objects
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}*/

void FileSystem::checkLogin(QString username, QString password, QTcpSocket *socket){

    QSqlQuery query;
    QVector<QString> files;
    QJsonArray files_array;

    query.prepare("SELECT rowid FROM password WHERE username = (:username) AND password = (:password)");
    query.bindValue(":username", username);
    QByteArray saltedPsw = password.append(STR_SALT_KEY).toUtf8();
    QString encryptedPsw = QString(QCryptographicHash::hash(saltedPsw, QCryptographicHash::Md5));
    query.bindValue(":password", encryptedPsw);
    //query.bindValue(":password", password);
    qDebug() << password << username;
    int id = -1;
    if (query.exec())
    {
        if (query.next())
        {
            id =  query.value("rowid").toInt();
        }
    }
    else{
        qDebug() << "Query not executed";
    }
    QJsonArray file_array;
    QJsonObject final_object;
    if(id != -1){
        sock_username.insert(std::pair<QTcpSocket*, QString> (socket, username)); //inserisco corrispondenza socket, username
        QSqlQuery query;
        sock_id.insert(std::pair<QTcpSocket*, int> (socket, id)); //associate id to socket
        query.prepare("SELECT filename, fileid FROM files WHERE userid = (:userid)");
        query.bindValue(":userid", id);
        if (query.exec())
        {
            while (query.next())
            {
               QJsonObject item_data;
               QString name = query.value("filename").toString();
               int fileid = query.value("fileid").toInt();
               qDebug() << name;
               item_data.insert("filename", QJsonValue(name));
               item_data.insert("fileid", QJsonValue(fileid));

               file_array.push_back(QJsonValue(item_data));
            }
            final_object.insert(QString("files"), QJsonValue(file_array));
        }
        else{
            qDebug() << "Query not executed";
        }
    }
    final_object.insert("id", QJsonValue(id));

    if(socket->state() == QAbstractSocket::ConnectedState){
        qDebug() << "Risposta al LOGIN:\n" << QJsonDocument(final_object).toJson().data();
        socket->write(QJsonDocument(final_object).toJson());
        socket->waitForBytesWritten(1000);
    }


}

void FileSystem::storeNewUser(QString username, QString psw, QTcpSocket *socket) {
    QSqlQuery sqlQuery;

    // check che username non sia gia' stato preso
    sqlQuery.prepare("SELECT COUNT(*) FROM PASSWORD WHERE username=(:username)");
    sqlQuery.bindValue(":username", username);
    int count = -1;
    if (sqlQuery.exec())
    {
        if (sqlQuery.next())
        {
            count = sqlQuery.value(0).toInt();
        }
    }
    else{
        qDebug() << "Query not executed";
        // EMIT SEGNALE PROBLEMA SERVER FAILED TO RESPOND
        emit signUpResponse("SERVER_FAILURE", false, socket);
        return;
    }
    if(count > 0){
        qDebug("Username already taken");
        // EMIT SEGNALE CAMBIA USERNAME
        emit signUpResponse("INVALID_USERNAME", false, socket);
        return;
    }

    // Encrypt password (sale + md5 hash)
    QByteArray saltedPsw = psw.append(STR_SALT_KEY).toUtf8();
    QString encryptedPsw = QString(QCryptographicHash::hash(saltedPsw, QCryptographicHash::Md5));

    /* Insert new user in DB */
    sqlQuery.prepare("INSERT INTO Password(username, password) VALUES ((:username),(:password))");    // safe for SQL injection
    sqlQuery.bindValue(":username", username);
    sqlQuery.bindValue(":password", encryptedPsw);
    //sqlQuery.bindValue(":password", psw);

    if (sqlQuery.exec()){
        // EMIT SIGN UP SUCCESSFUL
        emit signUpResponse("SUCCESS", true, socket);
    } else {
        qDebug() << "INSERT new user not executed!";
        // EMIT segnale server failed to respond
        emit signUpResponse("SERVER_FAILURE", false, socket);
        return;
    }
}


void FileSystem::sendInsert(QVector<QTcpSocket*> users, QByteArray message, bool modifiedIndex, int newIndex, QTcpSocket *client) {
    QJsonObject obj;
    if(modifiedIndex) {
        // Edit json file
        QJsonDocument jsonResponse = QJsonDocument::fromJson(message);
        QJsonObject rootObject = jsonResponse.object();
        obj.insert("type", "INSERT");
        obj.insert("filename", rootObject.value("filename").toString());
        obj.insert("letter", rootObject.value("letter").toString());
        obj.insert("position", rootObject.value("position").toArray());
        obj.insert("siteID", rootObject.value("siteID").toString());
        obj.insert("siteCounter", rootObject.value("siteCounter").toInt());
        obj.insert("externalIndex", newIndex);
    }
    QVectorIterator<QTcpSocket*> i(users);
    QByteArray sendSize;
    while (i.hasNext()){
        QTcpSocket* socket = i.next();
        if(socket == client) continue;
        if(socket->state() == QAbstractSocket::ConnectedState) {
            if(modifiedIndex) {
                QByteArray msg = QJsonDocument(obj).toJson();
                qDebug() << "Notifica inviata: " << msg.data();
                socket->write(sendSize.number(msg.size()), sizeof (long int));
                socket->waitForBytesWritten();
                socket->write(msg); //write size of data
            } else {
                qDebug() << "Notifica inviata: " << message.data();
                socket->write(sendSize.number(message.size()), sizeof (long int));
                socket->waitForBytesWritten();
                socket->write(message);
            }
            socket->waitForBytesWritten(1000);
            sendSize.clear();
        }
    }
}

void FileSystem::sendDelete(QVector<QTcpSocket*> users, QByteArray message, QTcpSocket* client){
    QVectorIterator<QTcpSocket*> i(users);
    QByteArray sendSize;

    while (i.hasNext()){
        QTcpSocket* socket = i.next();
        if(socket == client) continue;
        if(socket->state() == QAbstractSocket::ConnectedState) {
            socket->write(sendSize.number(message.size()), sizeof (long int));
            socket->waitForBytesWritten();
            socket->write(message);
            socket->waitForBytesWritten(1000);
        }
    }
}

void FileSystem::sendStyleChange(QVector<QTcpSocket*> users, QByteArray message, QTcpSocket* client) {
    QVectorIterator<QTcpSocket*> i(users);
    QByteArray sendSize;

    while (i.hasNext()){
        QTcpSocket* socket = i.next();
        if(socket == client) continue;
        if(socket->state() == QAbstractSocket::ConnectedState) {
            socket->write(sendSize.number(message.size()), sizeof (long int));
            socket->waitForBytesWritten();
            socket->write(message);
            socket->waitForBytesWritten(1000);
        }
    }
}

void FileSystem::updateFileSiteCounter(int fileID, int userID, int siteCounter){
    QSqlQuery query;
    query.prepare("UPDATE Files SET siteCounter=(:siteCounter) WHERE UserId=(:userID) AND FileId=(:fileID)");
    query.bindValue(":fileID", fileID);
    query.bindValue(":userID", userID);
    query.bindValue(":siteCounter", siteCounter);
    if(!query.exec()) {
        qDebug() << "Error! Site counter not updated.";
        return;
    }
    return;
}

std::map<int, FileHandler*> FileSystem::getFiles() {
    return this->files;
}

/*int FileSystem::getSocketID(QTcpSocket *socket){
    if(this->sock_id.(socket))
        return sock_id.value(socket);
    else return -1;
}*/

void FileSystem::disconnectClient(QTcpSocket* socket){
    int fileID = sock_file.at(socket);
    int userID = sock_id.at(socket);
    FileHandler *fh = files.at(fileID);
    this->updateFileSiteCounter(fileID, userID, fh->getSiteCounter(socket));
    fh->removeActiveUser(socket, sock_username.at(socket));
}

void FileSystem::sendJson(QJsonObject json, QTcpSocket* socket){

    if(socket->state() == QAbstractSocket::ConnectedState){
        qint32 msg_size = QJsonDocument(json).toJson().size();
        QByteArray toSend;
        socket->write(toSend.number(msg_size), sizeof (long int));
        socket->waitForBytesWritten();
        if(socket->write(QJsonDocument(json).toJson()) == -1){
            qDebug() << "File info failed to send";
            return;
        } //write the data itself
        socket->waitForBytesWritten();
    }

}

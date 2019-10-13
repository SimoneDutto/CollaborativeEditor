#include "filesystem.h"

// class to handle all files in a map

FileSystem *FileSystem::instance = nullptr;

static inline QByteArray IntToArray(qint32 source);

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

void FileSystem::sendFile(QString filename, QTcpSocket *socket){
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    auto it = files.find(filename);
    if (it != files.end()){
        // il file è già in memoria principale e può essere mandato
        // serializzarlo

        it->second->insertActiveUser(socket);


    }
    else{
        qDebug() << "Inizio l'invio del file";
        // apre il file, lo scrive in un DataStream che poi invierà
        QFile m_file (filename);
        m_file.open(QFile::ReadOnly);

        QByteArray q = m_file.readAll();
        if(socket->state() == QAbstractSocket::ConnectedState)
        {
            qDebug() << "Invio file";
            socket->write(IntToArray(q.size())); //write size of data
            if(socket->write(q) == -1){
                qDebug() << "File failed to send";
                return;
            } //write the data itself
            socket->waitForBytesWritten();
        }
        m_file.close();

        qDebug() << "File sent";

        QJsonDocument document = QJsonDocument::fromJson(q);
        QJsonObject object = document.object();
        QJsonValue value = object.value("letterArray");
        QJsonArray letterArray = value.toArray();

        QVector<Letter> letters;

        foreach (const QJsonValue& v, letterArray)
        {
            QChar letter = v.toObject().value("letter").toString().at(0);
            QString ID = v.toObject().value("externalIndex").toString();

            QJsonArray array_tmp = v.toObject().value("position").toArray();
            QVector<int> fractionals;
            for(auto fractional : array_tmp) {
                fractionals.append(fractional.toInt());
            }

            Letter letter_tmp = Letter(letter, fractionals, ID);
            letters.append(std::move(letter_tmp));
        }
        FileHandler *fh = new FileHandler(std::move(letters));
        fh->insertActiveUser(socket);

        files.insert(std::pair<QString, FileHandler*> (filename, fh));
        qDebug() << "File saved in the file system";
    }
}

QByteArray IntToArray(qint32 source) //Use qint32 to ensure that the number have 4 bytes
{
    //Avoid use of cast, this is the Qt way to serialize objects
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}

void FileSystem::checkLogin(QString username, QString password, QTcpSocket *socket){

    QSqlQuery query;
    QVector<QString> files;
    QJsonArray files_array;

    query.prepare("SELECT userid FROM password WHERE username = (:username) AND password = (:password)");
    query.bindValue(":username", username);
    query.bindValue(":password", password);
    int id = -1;
    if (query.exec())
    {
        if (query.next())
        {
            id =  query.value("userid").toInt();
        }
    }
    else{
        qDebug() << "Query not executed";
    }
    QJsonArray file_array;
    QJsonObject final_object;
    if(id != -1){
        QSqlQuery query;
        query.prepare("SELECT filename FROM files WHERE username = (:username)");
        query.bindValue(":username", username);
        if (query.exec())
        {
            while (query.next())
            {
               QJsonObject item_data;
               QString name = query.value("filename").toString();
               qDebug() << name;
               item_data.insert("filename", QJsonValue(name));

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

std::map<QString, FileHandler*> FileSystem::getFiles() {
    return this->files;
}



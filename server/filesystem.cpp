#include "filesystem.h"

// class to handle all files in a map

FileSystem *FileSystem::instance = nullptr;

static inline QByteArray IntToArray(qint32 source);

FileSystem* FileSystem::getInstance(){
    if(!instance){
        instance = new FileSystem();
    }
    return FileSystem::instance;
}

int FileSystem::sendFile(QString filename, QTcpSocket *socket){
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    auto it = files.find(filename);
    if (it != files.end()){
        // il file è già in memoria principale e può essere mandato
        // serializzarlo

    }
    else{
        qDebug() << "Inizio l'invio del file";
        // apre il file, lo scrive in un DataStream che poi invierà
        QFile *m_file = new QFile(filename);
        QByteArray q = m_file->readAll();

        if(socket->state() == QAbstractSocket::ConnectedState)
        {
            socket->write(IntToArray(q.size())); //write size of data
            if(socket->write(q) == -1){
                return -1;
            } //write the data itself
            socket->waitForBytesWritten();
        }
        m_file->close();

        QJsonDocument document = QJsonDocument::fromJson(q);
        QJsonObject object = document.object();
        QJsonValue value = object.value("letterArray");
        QJsonArray letterArray = value.toArray();

        QVector<Letter> fileLikeLetterArray;

        foreach (const QJsonValue& v, letterArray)
        {
           Letter letter_tmp = Letter(v.toObject().value("value").toString(),
                    v.toObject().value("id").toString(),
                    v.toObject().value("pos_intera").toInt(),
                    v.toObject().value("pos_decimale").toInt());

           fileLikeLetterArray.append(letter_tmp);
           //fileLikeLetterArray.append(std::move(letter_tmp));

           qDebug() << letter_tmp.getValue();
        }
        FileHandler *fh = new FileHandler(std::move(fileLikeLetterArray));
        fh->insertActiveUser(socket);
        // TODO:: da file a array di Letter con la deserializzazione
        files.insert(std::pair<QString, FileHandler*> (filename, fh));
    }
    return 0;
}

QByteArray IntToArray(qint32 source) //Use qint32 to ensure that the number have 4 bytes
{
    //Avoid use of cast, this is the Qt way to serialize objects
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}


#include "filesystem.h"
#include <QTcpSocket>
#include <QDataStream>
#include <QFile>


// class to handle all files in a map

FileSystem *FileSystem::instance = nullptr;

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
        // apre il file, lo scrive in un DataStream che poi invierà
        QFile *m_file = new QFile(filename);
        out.setVersion(QDataStream::Qt_5_4);

        out << (quint32)0 << m_file->fileName();

        QByteArray q = m_file->readAll();
        block.append(q);
        m_file->close();

        out.device()->seek(0);
        out << (quint32)(block.size() - sizeof(quint32));

        qint64 x = 0;
        while (x < block.size()) {
            qint64 y = socket->write(block);
            x += y;
            //qDebug() << x;    // summary size
        }
        // TODO:: da file a array di Letter con la deserializzazione
        //files.insert(std::pair<QString, FileHandler> (filename, ));
    }
    return 0;
}


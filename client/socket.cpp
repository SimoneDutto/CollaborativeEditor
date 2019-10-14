#include "socket.h"
#include "ui_socket.h"
#include <QDebug>

inline qint32 ArrayToInt(QByteArray source);

Socket::Socket(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Socket)
{
    ui->setupUi(this);
}

Socket::Socket(const QString &host, quint16 port)
{
    socket = new QTcpSocket(this);
    size = 0;
    connect( socket, SIGNAL(connected()), SLOT(socketConnected()) );
    connect( socket, SIGNAL(disconnected()), SLOT(socketConnectionClosed()));
    //connect( socket, SIGNAL(error(SocketError socketError)), SLOT(socketError(int)) );
    connect( socket, SIGNAL(readyRead()),  SLOT(checkAccountAndGetListFileName()));

    socket->connectToHost(host, port);

    if(socket->waitForConnected(3000))
    {
        qDebug() << "Connesso";
    }
    else {
        //NON CONNESSO
        qDebug() << "Non connesso";
    }
}

void Socket::socketConnected()
{
        qDebug() << "Connesso!\n";
}

void Socket::socketConnectionClosed()
{
        qDebug() << "Connection closed by the server\n";
}

void Socket::socketClosed()
{
        qDebug() << "Connection closed\n";
}

void Socket::socketError(int e)
{
        qDebug() << "Error number " << e <<" occurred\n";
}

Socket::~Socket()
{
    delete ui;
}

void Socket::closeConnection()
{
    socket->close();
    if ( socket->state() == QTcpSocket::ClosingState ) {
        // We have a delayed close.
        connect( socket, SIGNAL(delayedCloseFinished()),
                SLOT(socketClosed()) );
    } else {
        // The socket is closed.
        socketClosed();
    }
}

inline qint32 ArrayToInt(QByteArray source)
{
    qint32 temp;
    QDataStream data(&source, QIODevice::ReadWrite);
    data >> temp;
    return temp;
}



void Socket::checkLogin(QString username, QString password)
{
    //RICHIESTA
    QJsonObject obj;
    obj.insert("type", "LOGIN");
    obj.insert("nickname", username);
    obj.insert("password", password);

    if(socket->state() == QAbstractSocket::ConnectedState){
        qDebug() << "Richiesta:\n" << QJsonDocument(obj).toJson().data();
        socket->write(QJsonDocument(obj).toJson());
    }
    socket->waitForBytesWritten(1000);
}

void Socket::checkAccountAndGetListFileName()
{
    qDebug() << "Inizio a leggere i file a cui ho accesso e controllo i dati del login";
    QByteArray data = socket->readAll();
    QJsonDocument document = QJsonDocument::fromJson(data);
    QJsonObject object = document.object();

    QJsonValue value_id = object.value("id");
    clientID = value_id.toInt();
    if(clientID == -1){
        emit loginError();
        return;
    }

    this->fileh = new FileHandler(clientID);
    QJsonValue value = object.value("files");
    QJsonArray nameFilesArray = value.toArray();

    qDebug() << "Accessed files:";

    QVector<QString> listFiles_tmp;
    foreach (const QJsonValue& v, nameFilesArray)
    {
        QString q = v.toObject().value("filename").toString();
        qDebug() << q;
        listFiles_tmp.append(q);
    }
    this->fileh->setListFiles(listFiles_tmp);


    /*Setto le connect per gestire le notifiche che arrivano dal server*/
    disconnect(socket, SIGNAL(readyRead()), this, SLOT(checkAccountAndGetListFileName()));

    connect( socket, SIGNAL(readyRead()),  SLOT(notificationsHandler()));
    connect( socket, SIGNAL(readyInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter)),
             fileh,  SLOT(remoteInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter)));
    connect( socket, SIGNAL(readyDelete(QString deletedLetterID)),
             fileh, SLOT(remoteDelete(QString deletedLetterID)));
    connect( socket, SIGNAL(readyFile()), SLOT());

    emit loginSuccess();
    qDebug() << "Finished!";
    return;
}

void Socket::notificationsHandler(){
    qDebug() << "Leggo dal socket";

    QByteArray data;

    while (socket->bytesAvailable() > 0)
    {
       buffer.append(socket->readAll());
       while ((size == 0 && buffer.size() >= 4) || (size > 0 && buffer.size() >= size)) //While can process data, process it
       {
           if (size == 0 && buffer.size() >= 4) //if size of data has received completely, then store it on our global variable
           {
               size = ArrayToInt(buffer.mid(0, 4));
               buffer.remove(0, 4);
           }
           if (size > 0 && buffer.size() >= size) // If data has received completely, then emit our SIGNAL with the data
           {
               data = buffer.mid(0, size);
               buffer.remove(0, size);
               size = 0;
           }
       }
    }

    QJsonDocument document = QJsonDocument::fromJson(data);
    QJsonObject object = document.object();
    QString type = object.value("type").toString();
    qDebug() << "Tipo di richiesta: " << type;

    /*
     * OPEN
     * INSERT
     * DELETE
    */

    if(type.compare("OPEN")==0){
        QVector<Letter> letters;
        QJsonValue value = object.value("letterArray");
        QJsonArray letterArray = value.toArray();

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
            qDebug() << "Lettera:" << letter_tmp.getValue();
        }

        /*Salvo il file come vettore di Letters nel fileHandler*/
        this->fileh->setVectorLettersFile(letters);
        emit readyFile();
    }

    else if (type.compare("INSERT")==0) {
        QChar newLetterValue = object.value("letter").toString().at(0);
        QJsonArray position = object.value("position").toArray();
        int siteID = object.value("siteID").toInt();
        int siteCounter = object.value("siteCounter").toInt();
        int externalIndex = object.value("externalIndex").toInt();

        /*Inserire nel modello questa lettera e aggiornare la UI*/
        emit readyInsert(position, newLetterValue, externalIndex, siteID, siteCounter);
    }

    else if (type.compare("DELETE")==0) {
        QString deletedLetterID = object.value("letterID").toString();

        /*Cancellare dal modello questa lettera e aggiornare la UI*/
        qDebug() << "LettedID da cancellare: " << deletedLetterID;
        emit readyDelete(deletedLetterID);
    }

    qDebug() << "Finished!";
    return;
}

int Socket::sendInsert(int pos, QString value)
{
    /*RICHIESTA*/
    QJsonObject obj;
    obj.insert("type", "INSERT");
    obj.insert("value", value);
    obj.insert("position", pos);

    if(socket->state() == QAbstractSocket::ConnectedState){
        qDebug() << "Richiesta:\n" << QJsonDocument(obj).toJson().data();
        socket->write(QJsonDocument(obj).toJson());
    }

    return socket->waitForBytesWritten(1000);
}

void Socket::sendCheckFileName(QString fileNameTmp){
    //RICHIESTA
    QJsonObject obj;
    obj.insert("type", "CHECKFILENAME");
    obj.insert("filename", fileNameTmp);

    if(socket->state() == QAbstractSocket::ConnectedState){
        qDebug() << "Richiesta:\n" << QJsonDocument(obj).toJson().data();
        socket->write(QJsonDocument(obj).toJson());
    }
    socket->waitForBytesWritten(1000);
}

int Socket::sendOpenFile(QString name_file)
{
    /*RICHIESTA*/
    QJsonObject obj;
    obj.insert("type", "OPEN");
    obj.insert("filename", name_file);

    //disconnect(socket, SIGNAL(readyRead()), this, SLOT(socketReadyReadFile()));
    //connect( socket, SIGNAL(readyRead()),  SLOT(socketReadyReadFile()));

    if(socket->state() == QAbstractSocket::ConnectedState){
        qDebug() << "Richiesta:\n" << QJsonDocument(obj).toJson().data();
        socket->write(QJsonDocument(obj).toJson());
    }

    return socket->waitForBytesWritten(1000);
}

void Socket::updateLocalInsert(int externalIndex, QChar newLetterValue){
    this->fileh->localInsert(externalIndex, newLetterValue, this->clientID);
}

FileHandler* Socket::getFHandler(){
    return this->fileh;
}

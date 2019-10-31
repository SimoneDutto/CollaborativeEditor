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
    connect( socket, SIGNAL(disconnected()), SLOT(socketConnectionClosed()) );
    //connect( socket, SIGNAL(error(SocketError socketError)), SLOT(socketError(int)) );
    connect( socket, SIGNAL(readyRead()),  SLOT(checkLoginAndGetListFileName()) );

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

FileHandler* Socket::getFHandler(){
    return this->fileh;
}

int Socket::getClientID(){
    return this->clientID;
}



void Socket::sendLogin(QString username, QString password)
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

void Socket::checkLoginAndGetListFileName()
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
    /*connect della DELETE dovrebbe essere giusta, quella della INSERT serve capire cosa server al server*/
    connect( this->fileh, SIGNAL(localInsertNotify(QChar, QJsonArray, int, int, int)),
             this, SLOT(sendInsert(QChar, QJsonArray, int, int, int)) );
    connect( this->fileh, SIGNAL(localDeleteNotify(int)), this, SLOT(sendDelete(int)) );

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


    /*Le connect per gestire le notifiche che arrivano dal server le setto nel costruttore di MainWindow*/
    disconnect(socket, SIGNAL(readyRead()), this, SLOT(checkLoginAndGetListFileName()));
    connect(socket, SIGNAL(readyRead()),  SLOT(notificationsHandler()));

    emit loginSuccess();
    qDebug() << "Finished!";
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
     * CHECKNAME
    */

    if(type.compare("OPEN")==0){
        QVector<Letter*> letters;
        QJsonValue value = object.value("letterArray");
        QJsonArray letterArray = value.toArray();

        foreach (const QJsonValue& v, letterArray)
        {
            QChar letter = v.toObject().value("letter").toString().at(0);
            QString ID = v.toObject().value("letterID").toString();

            QJsonArray array_tmp = v.toObject().value("position").toArray();
            QVector<int> fractionals;
            for(auto fractional : array_tmp) {
                fractionals.append(fractional.toInt());
            }

            letters.append(std::move(new Letter(letter, fractionals, ID)));
            qDebug() << "Lettera:" << letter;
        }

        /*Salvo il file come vettore di Letters nel fileHandler*/
        this->fileh->setVectorLettersFile(std::move(letters));
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

    else if (type.compare("CHECKNAME")==0) {

    }

    qDebug() << "Finished!";
}

int Socket::sendInsert(QChar newLetterValue, QJsonArray position, int siteID, int siteCounter, int externalIndex)
{
    /*RICHIESTA*/
    QJsonObject obj;
    obj.insert("type", "INSERT");
    obj.insert("filename", this->fileh->getFileName());
    obj.insert("letter", QJsonValue(newLetterValue));
    obj.insert("position", position);
    obj.insert("siteID", siteID);
    obj.insert("siteCounter", siteCounter);
    obj.insert("externalIndex", externalIndex);

    if(socket->state() == QAbstractSocket::ConnectedState){
        qDebug() << "Richiesta:\n" << QJsonDocument(obj).toJson().data();
        socket->write(QJsonDocument(obj).toJson());
    }

    return socket->waitForBytesWritten(1000);
}

int Socket::sendDelete(int externalIndex){
    /*RICHIESTA*/
    QJsonObject obj;
    obj.insert("type", "DELETE");
    obj.insert("externalIndex", externalIndex);

    if(socket->state() == QAbstractSocket::ConnectedState){
        qDebug() << "Richiesta:\n" << QJsonDocument(obj).toJson().data();
        socket->write(QJsonDocument(obj).toJson());
    }

    return socket->waitForBytesWritten(1000);
}

int Socket::sendCheckFileName(QString fileNameTmp){
    //RICHIESTA
    QJsonObject obj;
    obj.insert("type", "CHECKFILENAME");
    obj.insert("filename", fileNameTmp);

    if(socket->state() == QAbstractSocket::ConnectedState){
        qDebug() << "Richiesta:\n" << QJsonDocument(obj).toJson().data();
        socket->write(QJsonDocument(obj).toJson());
    }
    return socket->waitForBytesWritten(1000);
}

int Socket::sendOpenFile(QString name_file)
{
    /*Salvo il nome del file che sto aprendo*/
    this->fileh->setFileName(name_file);

    /*RICHIESTA*/
    QJsonObject obj;
    obj.insert("type", "OPEN");
    obj.insert("filename", name_file);

    if(socket->state() == QAbstractSocket::ConnectedState){
        qDebug() << "Richiesta:\n" << QJsonDocument(obj).toJson().data();
        socket->write(QJsonDocument(obj).toJson());
    }

    return socket->waitForBytesWritten(1000);
}

int Socket::sendNewFile(){
    /*RICHIESTA*/
    QJsonObject obj;
    obj.insert("type", "NEWFILE");

    if(socket->state() == QAbstractSocket::ConnectedState){
        qDebug() << "Richiesta:\n" << QJsonDocument(obj).toJson().data();
        socket->write(QJsonDocument(obj).toJson());
    }

    return socket->waitForBytesWritten(1000);
}

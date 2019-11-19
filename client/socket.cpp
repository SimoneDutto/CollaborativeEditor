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
    fileh = new FileHandler();

    /*Setto le connect del socket*/
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



void Socket::sendSignUpRequest(QString username, QString password) {
    // RICHIESTA DI REGISTRAZIONE NUOVO UTENTE
    QJsonObject obj;
    obj.insert("type", "SIGNUP");
    obj.insert("username", username);
    obj.insert("password", password);
    if(socket->state() == QAbstractSocket::ConnectedState){
        qDebug() << "Richiesta di registrazione:\n" << QJsonDocument(obj).toJson().data();
        socket->write(QJsonDocument(obj).toJson());
        socket->waitForBytesWritten(1000);
    }
}


void Socket::sendLogin(QString username, QString password)
{
    //RICHIESTA DI LOGIN
    QJsonObject obj;
    obj.insert("type", "LOGIN");
    obj.insert("nickname", username);
    obj.insert("password", password);

    if(socket->state() == QAbstractSocket::ConnectedState){
        qDebug() << "Richiesta:\n" << QJsonDocument(obj).toJson().data();
        socket->write(QJsonDocument(obj).toJson());
        socket->waitForBytesWritten(1000);
    }
}

void Socket::checkLoginAndGetListFileName()
{
    qDebug() << "Inizio a leggere i file a cui ho accesso e controllo i dati del login";
    QByteArray data = socket->readAll();
    QJsonDocument document = QJsonDocument::fromJson(data);
    QJsonObject object = document.object();

    clientID = object.value("id").toInt();
    if(clientID == -1){
        emit loginError();
        return;
    }

    QJsonValue value = object.value("files");
    QJsonArray nameFilesArray = value.toArray();

    qDebug() << "Accessed files:";

    QVector<QString> listFiles_tmp;
    foreach (const QJsonValue& v, nameFilesArray)
    {
        QString filename = v.toObject().value("filename").toString();
        int fileid = v.toObject().value("fileid").toInt();

        this->mapFiles.insert(filename, fileid);
    }

    /*Le connect per gestire le notifiche che arrivano dal server le setto nel costruttore di MainWindow*/
    disconnect(socket, SIGNAL(readyRead()), this, SLOT(checkLoginAndGetListFileName()));
    connect(socket, SIGNAL(readyRead()),  SLOT(notificationsHandler()));

    emit loginSuccess();
    qDebug() << "Finished!";
}



void Socket::notificationsHandler(){
    qDebug() << "Leggo dal socket";

    QByteArray buffer;

    buffer.append(socket->readAll());
    qDebug() << buffer;
    QJsonDocument document = QJsonDocument::fromJson(buffer);
    QJsonObject object = document.object();
    QString type = object.value("type").toString();
    qDebug() << "Tipo di richiesta: " << type;

    /*
     * OPEN
     * INSERT
     * DELETE
     * CHECKNAME
     * SIGNUP_RESPONSE
    */

    if(type.compare("OPEN")==0){

        int fileid = object.value("fileid").toInt();

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

        /*Creo il FileHandler*/
        connect( this->fileh, SIGNAL(localInsertNotify(QChar, QJsonArray, int, int, int)),
                 this, SLOT(sendInsert(QChar, QJsonArray, int, int, int)) );
        connect( this->fileh, SIGNAL(localDeleteNotify(int)), this, SLOT(sendDelete(int)) );

        /*Salvo il file come vettore di Letters nel fileHandler*/
        this->fileh->setValues(std::move(letters), fileid);
        emit readyFile();   //CREA CRASH QUANDO CHIAMA MAINWINDOW
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
        qDebug() << "LetterID da cancellare: " << deletedLetterID;
        emit readyDelete(deletedLetterID);
    }

    else if (type.compare("NEW")==0) {
        qDebug() << "Il file è stato creato correttamente!";
    }
    else if (type.compare("SIGNUP_RESPONSE")==0) {
        bool successful = object.value("success").toBool();
        QString message = object.value("msg").toString();
        if(!successful) {
            if(message.compare("INVALID_USERNAME"))
                emit invalidUsername();
            // emit segnale sign up not successful
            else if (message.compare("SERVER_FAILURE"))
                emit signUpError();
        } else
            // emit segnale sign up successful
            emit signUpSuccess();
    }

    qDebug() << "Finished!";
}

int Socket::sendInsert(QChar newLetterValue, QJsonArray position, int siteID, int siteCounter, int externalIndex)
{
    /*RICHIESTA*/
    QJsonObject obj;
    obj.insert("type", "INSERT");
    obj.insert("fileid", this->fileh->getFileId());
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

int Socket::sendOpenFile(QString filename)
{
    /*RICHIESTA*/
    QJsonObject obj;
    obj.insert("type", "OPEN");
    obj.insert("fileid", this->mapFiles.value(filename));

    if(socket->state() == QAbstractSocket::ConnectedState){
        qDebug() << "Richiesta:\n" << QJsonDocument(obj).toJson().data();
        socket->write(QJsonDocument(obj).toJson());
    }

    return socket->waitForBytesWritten(1000);
}

int Socket::sendNewFile(QString filename){
    /*RICHIESTA*/
    QJsonObject obj;
    obj.insert("type", "NEW");
    obj.insert("filename", filename);

    if(socket->state() == QAbstractSocket::ConnectedState){
        qDebug() << "Richiesta:\n" << QJsonDocument(obj).toJson().data();
        socket->write(QJsonDocument(obj).toJson());
    }

    return socket->waitForBytesWritten(1000);
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

QMap<QString, int> Socket::getMapFiles(){
    return this->mapFiles;
}

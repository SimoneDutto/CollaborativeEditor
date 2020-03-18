#include "socket.h"
#include "ui_socket.h"
#include <QDebug>
#include <QtEndian>
#include <QDataStream>

inline qint32 ArrayToInt(QByteArray source);
extern int nLogin = 0;

Socket::Socket(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Socket)
{
    ui->setupUi(this);
}

Socket::Socket(const QString &host, quint16 port)
{
    socket = new QTcpSocket(this);
    fileh = new FileHandler();

    /* Setto le connect del socket */
    connect( socket, SIGNAL(connected()), SLOT(socketConnected()) );
    connect( socket, SIGNAL(disconnected()), SLOT(socketConnectionClosed()) );
    //connect( socket, SIGNAL(error(SocketError socketError)), SLOT(socketError(int)) );
    // Qui NO connect per login: se login fallisce, non si riconnette il segnale
    connect( socket, SIGNAL(readyRead()),  SLOT(checkLoginAndGetListFileName()) , Qt::UniqueConnection);

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

/*void Socket::setSignals() {
    disconnect( socket, SIGNAL(readyRead()), this, SLOT(setSignals()) );

    if(this->isDoingSignUp)
        connect( socket, SIGNAL(bufferReady(QByteArray)),  SLOT(checkSignUp(QByteArray)) );
    else connect( socket, SIGNAL(bufferReady(QByteArray)),  SLOT(checkLoginAndGetListFileName(QByteArray)) );

    QByteArray data = socket->readAll();
    emit bufferReady(data);
}*/


void Socket::sendSignUpRequest(QString username, QString password, QString pathUserImage) {
    // RICHIESTA DI REGISTRAZIONE NUOVO UTENTE
    QJsonObject obj;
    QByteArray toSend;
    obj.insert("type", "SIGNUP");
    obj.insert("username", username);
    obj.insert("password", password);
    if(socket->state() == QAbstractSocket::ConnectedState){
        qDebug() << "Richiesta di registrazione:\n" << QJsonDocument(obj).toJson().data();
        socket->write(toSend.number(QJsonDocument(obj).toJson().size()), sizeof(long int));
        socket->waitForBytesWritten();
        socket->write(QJsonDocument(obj).toJson());
        socket->waitForBytesWritten(1000);
    }
    if(nLogin == 0) {
        disconnect( socket, SIGNAL(readyRead()), this, SLOT(checkLoginAndGetListFileName()) );
        connect( socket, SIGNAL(readyRead()),  this, SLOT(checkSignUp()), Qt::UniqueConnection);
    }
    nLogin++;
}

void Socket::checkSignUp() {
    QByteArray data = socket->readAll();
    QJsonDocument document = QJsonDocument::fromJson(data);
    QJsonObject object = document.object();
    QString type = object.value("type").toString();
    qDebug() << "Tipo di richiesta: " << type;

    if (type.compare("SIGNUP_RESPONSE")==0) {
        bool successful = object.value("success").toBool();
        QString message = object.value("msg").toString();
        if(!successful) {
            //connect(socket, SIGNAL(readyRead()), this, SLOT(checkSignUp()));
            if(message.compare("INVALID_USERNAME") == 0)
                emit invalidUsername();
            else if (message.compare("SERVER_FAILURE") == 0) // emit segnale sign up not successful
                emit signUpError();
        } else {
            // emit segnale sign up successful
            qDebug() << "Sign up successful";
            //connect(socket, SIGNAL(readyRead()), SLOT(readBuffer()));
            //connect(this, SIGNAL(bufferReady(QByteArray)), SLOT(notificationsHandler(QByteArray))
            // NO QUESTE CONNECT: vengono fatte dopo al login
            disconnect(socket, SIGNAL(readyRead()), this, SLOT(checkSignUp()));
            connect( socket, SIGNAL(readyRead()), this, SLOT(checkLoginAndGetListFileName()) , Qt::UniqueConnection);

            emit signUpSuccess();
        }
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
        QByteArray qarray = QJsonDocument(obj).toJson();
        qint32 msg_size = qarray.size();
        QByteArray toSend;
        socket->write(toSend.number(msg_size), sizeof (long int));
        socket->waitForBytesWritten();
        socket->write(QJsonDocument(obj).toJson());
        socket->waitForBytesWritten(1000);
    }
}

void Socket::sendAccess(QString URI){
    QJsonObject obj;

    obj.insert("type", "ACCESS");
    obj.insert("URI", URI);
    if(socket->state() == QAbstractSocket::ConnectedState){
        QByteArray qarray = QJsonDocument(obj).toJson();
        qint32 msg_size = qarray.size();
        QByteArray toSend;
        socket->write(toSend.number(msg_size), sizeof (long int));
        socket->waitForBytesWritten();
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
        //disconnect(socket, SIGNAL(readyRead()), this, SLOT(checkLoginAndGetListFileName())); SE DISCONNETTO, NON VIENE CHIAMATA LA FUNZIONE QUANDO VENGONO INSERITI NUOVI DATI
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
    connect(socket, SIGNAL(readyRead()), SLOT(readBuffer()));
    connect(this, SIGNAL(bufferReady(QByteArray)), SLOT(notificationsHandler(QByteArray)));
    //connect(socket, SIGNAL(myReadyRead()), SLOT(notificationsHandler()));

    emit loginSuccess();
    qDebug() << "Finished!";
}

void Socket::readBuffer(){

    QByteArray buffer_size, data;

    while (socket->bytesAvailable() > 0 || buffer.size() != 0 )
    {
        qDebug() << "Leggo dal socket";
       buffer.append(socket->readAll());
       while ((size == 0 && buffer.size() >= 8) || (size > 0 && buffer.size() >= size)) //While can process data, process it
       {
           if (size == 0 && buffer.size() >= 8) //if size of data has received completely, then store it on our global variable
           {
               size = atoi(buffer.mid(0, 8).data());
               qDebug() << "Size: " << size;
               buffer.remove(0, 8);
           }
           if (size > 0 && buffer.size() >= static_cast<int>(size)) // If data has received completely, then emit our SIGNAL with the data
           {
               data = buffer.mid(0, static_cast<int>(size));
               buffer.remove(0,static_cast<int>(size));
               size = 0;
               qDebug() << "Data: " << data.data();
               emit bufferReady(data);
           }
       }
    }
}

void Socket::notificationsHandler(QByteArray data){
    QJsonDocument document = QJsonDocument::fromJson(data);
    QJsonObject object = document.object();
    QString type = object.value("type").toString();
    qDebug() << "Tipo di richiesta: " << type;

    /*
     * OPEN
     * FILE
     * INSERT
     * DELETE
     * NEW
     * CHECKNAME
     * SIGNUP_RESPONSE
     * STYLE
    */

    if(type.compare("OPEN")==0){
        this->fileh->setFileId(object.value("fileid").toInt());
        this->fileh->setSize(object.value("size").toInt());
        this->fileh->setSiteCounter(object.value("siteCounter").toInt());
        emit writeURI(object.value("URI").toString());
        QJsonArray array_tmp = object.value("activeUser").toArray();
        for(auto user : array_tmp) {
            QString username = user.toString();
            QColor random = QColor(rand()%255, rand()%255, rand()%255, rand()%255);
            userColor.insert(username, random);
            emit UserConnect(username, random);
        }
        // fileid < 0 non puoi aprire il file
    }
    else if(type.compare("FILE")==0){
        int remaining = object.value("remaining").toInt();
        QString chunk = object.value("chunk").toString();
        json_buffer.append(chunk);
        if(remaining == 0){
            qDebug() << "Whole file: " << json_buffer;
            QJsonDocument doc = QJsonDocument::fromJson(json_buffer.data());
            QJsonObject full_chunks = doc.object();
            QVector<Letter*> letters;
            QJsonValue value = full_chunks.value("letterArray");
            QJsonArray letterArray = value.toArray();
            qDebug().noquote() << json_buffer.data();

            foreach (const QJsonValue& v, letterArray)
            {
                QChar letter = v.toObject().value("letter").toString().at(0);
                QString ID = v.toObject().value("letterID").toString();

                QJsonArray array_tmp = v.toObject().value("position").toArray();
                QVector<int> fractionals;
                for(auto fractional : array_tmp) {
                    fractionals.append(fractional.toInt());
                }

                /* Estrarre formato lettera */
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

                letters.append(std::move(new Letter(letter, fractionals, ID, format)));
                //qDebug() << "Lettera:" << letter;
            }
            json_buffer.clear();
          
            /*Creo il FileHandler*/
            connect( this->fileh, SIGNAL(localInsertNotify(QChar, QJsonArray, int, int, int, QTextCharFormat)),
                     this, SLOT(sendInsert(QChar, QJsonArray, int, int, int, QTextCharFormat)) );
            connect( this->fileh, SIGNAL(localDeleteNotify(QString, int, int)), this, SLOT(sendDelete(QString, int, int)) );
            connect( this->fileh, SIGNAL(localStyleChangeNotify(QString, QString, int, QString)),
                     this, SLOT(sendChangeStyle(QString, QString, int, QString)));

            /*Salvo il file come vettore di Letters nel fileHandler*/
            this->fileh->setValues(std::move(letters));
            emit readyFile();
        }

    }
    else if (type.compare("INSERT")==0) {
        QChar newLetterValue = object.value("letter").toString().at(0);
        QJsonArray position = object.value("position").toArray();
        int siteID = object.value("siteID").toInt();
        int siteCounter = object.value("siteCounter").toInt();
        int externalIndex = object.value("externalIndex").toInt();

        /* Estrarre formato lettera */
        QTextCharFormat format;
        bool isBold = object.value("isBold").toBool();
        bool isItalic = object.value("isItalic").toBool();
        bool isUnderlined = object.value("isUnderlined").toBool();

        if(isBold)
            format.setFontWeight(75);
        else format.setFontWeight(50);
        if(isItalic)
            format.setFontItalic(true);
        else format.setFontItalic(false);
        if(isUnderlined)
            format.setFontUnderline(true);
        else format.setFontUnderline(false);


        /*Inserire nel modello questa lettera e aggiornare la UI*/
        emit readyInsert(position, newLetterValue, externalIndex, siteID, siteCounter, format);
        
    }

    else if (type.compare("DELETE")==0) {
        QString deletedLetterID = object.value("letterID").toString();
        int siteCounter = object.value("siteCounter").toInt();
        /*Cancellare dal modello questa lettera e aggiornare la UI*/
        qDebug() << "LetterID da cancellare: " << deletedLetterID;
        emit readyDelete(deletedLetterID);
    }

    else if (type.compare("NEW")==0) {
        int id = object.value("fileid").toInt();
        if(id != -1){
            this->fileh->setFileId(id);
            this->fileh->setSize(0);
            this->fileh->getVectorFile().clear();
            qDebug() << "Il file è stato creato correttamente!";
        }
        else{
            qDebug() << "Il File non è stato creato";
        }
    }

    else if(type.compare("STYLE")==0) {
        int fileID = object.value(("fileid")).toInt();
        QString initialIndex = object.value("startIndex").toString();
        QString lastIndex = object.value("lastIndex").toString();
        QString changedStyle = object.value("changedStyle").toString();
        emit readyStyleChange(initialIndex, lastIndex, changedStyle);
    }
    else if(type.compare("USER_CONNECT")==0){
        QString username = object.value("username").toString();
        QColor random = QColor(rand()%255, rand()%255, rand()%255, rand()%255);
        userColor.insert(username, random);
        emit UserConnect(username, random);
    }
    else if(type.compare("USER_DISCONNECT")==0){
        QString username = object.value("username").toString();
        userColor.remove(username);
        emit UserDisconnect(username);
    }
    else if(type.compare("ACCESS_RESPONSE")==0){
            int fileid = object.value("fileid").toInt();
            QString filename = object.value("filename").toString();
            if(fileid > 0){
                this->mapFiles.insert(filename, fileid);
                emit uriIsOk(filename);
            }
            else{
                emit uriIsNotOk();
            }
        }
    /*else if (type.compare("SIGNUP_RESPONSE")==0) {
        bool successful = object.value("success").toBool();
        QString message = object.value("msg").toString();
        if(!successful) {
            if(message.compare("INVALID_USERNAME"))
                emit invalidUsername();
            else if (message.compare("SERVER_FAILURE")) // emit segnale sign up not successful
                emit signUpError();
        } else
            // emit segnale sign up successful
            qDebug() << "Sign up successful";
            emit signUpSuccess();
    }*/
    //if(socket->bytesAvailable())
      //  emit myReadyRead();
    qDebug() << "Finished!";
    emit socket->readyRead();
}


int Socket::sendInsert(QChar newLetterValue, QJsonArray position, int siteID, int siteCounter, int externalIndex, QTextCharFormat format)
{
    /*RICHIESTA*/
    QJsonObject obj;
    obj.insert("type", "INSERT");
    obj.insert("fileid", this->fileh->getFileId());
    obj.insert("letter", QJsonValue(newLetterValue));
    obj.insert("position", position);
    obj.insert("isBold", QJsonValue(format.fontWeight()==75));
    obj.insert("isItalic", QJsonValue(format.fontItalic()));
    obj.insert("isUnderlined", QJsonValue(format.fontUnderline()));
    obj.insert("siteID", siteID);
    obj.insert("siteCounter", siteCounter);
    obj.insert("externalIndex", externalIndex);

    // Aggiungere il formato
    //obj.insert("style", style);


    if(socket->state() == QAbstractSocket::ConnectedState){
        QByteArray qarray = QJsonDocument(obj).toJson();
        qint32 msg_size = qarray.size();
        QByteArray toSend;
        socket->write(toSend.number(msg_size), sizeof (long int));
        socket->waitForBytesWritten();
        socket->write(QJsonDocument(obj).toJson());
        socket->waitForBytesWritten();
        qDebug() << "Richiesta:\n" << QJsonDocument(obj).toJson().data();
    }

    return socket->waitForBytesWritten(1000);
}

int Socket::sendDelete(QString deletedLetterID, int fileID, int siteCounter){
    /*RICHIESTA*/
    QJsonObject obj;
    obj.insert("type", "DELETE");
    obj.insert("fileid", fileID);
    obj.insert("letterID", deletedLetterID);
    obj.insert("siteCounter", siteCounter);
    //obj.insert("externalIndex", externalIndex);

    if(socket->state() == QAbstractSocket::ConnectedState){
        QByteArray qarray = QJsonDocument(obj).toJson();
        qint32 msg_size = qarray.size();
        QByteArray toSend;
        socket->write(toSend.number(msg_size), sizeof (long int));
        socket->waitForBytesWritten();
        socket->write(qarray);
        socket->waitForBytesWritten();
        qDebug() << "Richiesta:\n" << qarray.data();
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
        QByteArray qarray = QJsonDocument(obj).toJson();
        qint32 msg_size = qarray.size();
        QByteArray toSend;
        socket->write(toSend.number(msg_size), sizeof (long int));
        socket->waitForBytesWritten();
        socket->write(QJsonDocument(obj).toJson());
        socket->waitForBytesWritten();
        qDebug() << "Richiesta:\n" << QJsonDocument(obj).toJson().data();
    }

    return socket->waitForBytesWritten(1000);
}

int Socket::sendNewFile(QString filename){
    /*RICHIESTA*/
    QJsonObject obj;
    obj.insert("type", "NEW");
    obj.insert("filename", filename);

    if(socket->state() == QAbstractSocket::ConnectedState){
        QByteArray qarray = QJsonDocument(obj).toJson();
        qint32 msg_size = qarray.size();
        QByteArray toSend;
        socket->write(toSend.number(msg_size), sizeof (long int));
        socket->waitForBytesWritten();
        socket->write(QJsonDocument(obj).toJson());
        socket->waitForBytesWritten();
        qDebug() << "Richiesta:\n" << QJsonDocument(obj).toJson().data();
    }

    return socket->waitForBytesWritten(1000);
}

int Socket::sendChangeStyle(QString firstLetterID, QString lastLetterID, int fileID, QString changedStyle){
    /* Notificare il cambiamento di stile */
    //QString startID = letterFormatMap.firstKey();
    //QString endID = letterFormatMap.lastKey();

    QJsonObject obj;
    obj.insert("type", "STYLE");
    obj.insert("fileid", fileID);
    obj.insert("startIndex", firstLetterID);
    obj.insert("lastIndex", lastLetterID);
    obj.insert("changedStyle", changedStyle);

    if(socket->state() == QAbstractSocket::ConnectedState){
        QByteArray qarray = QJsonDocument(obj).toJson();
        qint32 msg_size = qarray.size();
        QByteArray toSend;
        socket->write(toSend.number(msg_size), sizeof (long int));
        socket->waitForBytesWritten();
        socket->write(QJsonDocument(obj).toJson());
        socket->waitForBytesWritten();
        qDebug() << "Richiesta:\n" << QJsonDocument(obj).toJson().data();
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

void Socket::isSigningUp(bool flag) {
    this->isDoingSignUp = flag;
}

QMap<QString, int> Socket::getMapFiles(){
    return this->mapFiles;
}

QMap<QString, QColor> Socket::getUserColor(){
    return this->userColor;
}

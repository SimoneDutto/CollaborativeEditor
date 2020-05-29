#include "socket.h"
#include "ui_socket.h"
#include <QDebug>
#include <QtEndian>
#include <QDataStream>
#include <QImageWriter>

#define DATA_SIZE 1024

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
    fileh = new FileHandler();

    /* Setto le connect del socket */
    connect( socket, SIGNAL(connected()), SLOT(socketConnected()) );
    connect( socket, SIGNAL(disconnected()), SLOT(socketConnectionClosed()) );
    //connect( socket, SIGNAL(error(SocketError socketError)), SLOT(socketError(int)) );
    // Qui NO connect per login: se login fallisce, non si riconnette il segnale
    //connect( socket, SIGNAL(readyRead()),  SLOT(checkLoginAndGetListFileName()) , Qt::UniqueConnection);
    connect(socket, SIGNAL(readyRead()), SLOT(readBuffer()));
    connect(this, SIGNAL(bufferReady(QByteArray)), SLOT(notificationsHandler(QByteArray)));
    connect(this, SIGNAL(noConnection()), this, SLOT(notConnected()));

    socket->connectToHost(host, port);

    if(socket->waitForConnected(3000))
    {
        qDebug() << "Connesso";
        connected=true;
    }
    else {
        //NON CONNESSO
        qDebug() << "Non connesso";
        connected=false;
        emit notConnected();
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

void Socket::sendChange(QString psw, QString path){
    QJsonObject obj;
    QByteArray toSend;
    if(psw.compare("")!=0){
        obj.insert("type", "CHANGE");
        obj.insert("password", psw);
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
    if(path.compare("null")!=0) sendIcon(path);
}

void Socket::sendSignUpRequest(QString username, QString password, QString pathUserImage) {
    // RICHIESTA DI REGISTRAZIONE NUOVO UTENTE
    QJsonObject obj;
    QByteArray toSend;
    obj.insert("type", "SIGNUP");
    obj.insert("username", username);
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
    pathIcon = pathUserImage;
}
void Socket::sendIcon(QString path){
    QFile inFile(path);
    inFile.open(QFile::ReadOnly);
    QByteArray splitToSend = inFile.readAll().toBase64();
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
            sendFileChunk(splitToSend.mid(from, chunk), socket, remaining);
        else if (remaining == 0)
            sendFileChunk(splitToSend.mid(from, chunk+1), socket, remaining);
        from += chunk;
    }
    qDebug() << "Icon sent";
}
void Socket::sendFileChunk(QByteArray chunk, QTcpSocket* socket, int remainingSize) {
    QJsonObject object;
    QByteArray toSend;
    QString s_data;
    s_data = chunk.data();
    object.insert("type", "ICON");
    object.insert("chunk", s_data);
    object.insert("remaining", remainingSize);
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        //qDebug() << "Invio file";
        qDebug() << "size: " << QJsonDocument(object).toJson().size();
        qDebug() << "file with content: " << object;
        socket->write(toSend.number(QJsonDocument(object).toJson().size()), sizeof (long int));
        socket->waitForBytesWritten();
        socket->write(QJsonDocument(object).toJson());
        socket->waitForBytesWritten();
    }
}

void Socket::checkSignUp(QJsonObject object) {
    bool successful = object.value("success").toBool();
    QString message = object.value("msg").toString();
    if(!successful) {
        if(message.compare("INVALID_USERNAME") == 0)
            emit invalidUsername();
        else if (message.compare("SERVER_FAILURE") == 0) // emit segnale sign up not successful
            emit signUpError();
    } else {
        // emit segnale sign up successful
        qDebug() << "Sign up successful";
        emit signUpSuccess();
        sendIcon(pathIcon);
    }
}

void Socket::sendLogin(QString username, QString password)
{
    /* Salvo il nome utente */
    this->username = username;

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

void Socket::sendHistory(){
    QJsonObject obj;
    obj.insert("type", "HISTORY");
    obj.insert("fileid", this->fileh->getFileId());
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

void Socket::checkLoginAndGetListFileName(QJsonObject object)
{
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
    emit loginSuccess();
    qDebug() << "Finished!";
}

void Socket::getUsernames(QJsonObject object){
    QJsonValue value = object.value("usernames");
    QJsonArray nameFilesArray = value.toArray();
    qDebug() << "Usernames:";
    QVector<QString> listFiles_tmp;
    QMap<int, QString> mapUsername;

    foreach (const QJsonValue& v, nameFilesArray)
    {
        QString username = v.toObject().value("username").toString();
        int userid = v.toObject().value("rowid").toInt();
        qDebug() << username;
        mapUsername.insert(userid, username);
    }
    emit HistorySuccess(mapUsername);
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
     * ALIGNMENT
     * COLOR
     * USER_CONNECT
     * USER_DISCONNECT
     * CURSOR
     * LOGIN
     * SIGNUP_RESPONSE
     * HISTORY
    */

    if(type.compare("OPEN")==0){
        this->fileh->setFileId(object.value("fileid").toInt());
        this->fileh->setSize(object.value("size").toInt());
        this->fileh->setSiteCounter(object.value("siteCounter").toInt());
        emit writeURI(object.value("URI").toString());
        if(object.contains("activeUser") && object.contains("activeUserIDs") && object.contains("userCursors")) {
            QJsonArray array_tmp = object.value("activeUser").toArray();
            QJsonArray userIDs = object.value("activeUserIDs").toArray();
            QJsonArray userCursors = object.value("userCursors").toArray();
            int i = 0;
            // reset older values
            if(!userColor.isEmpty())
                userColor.clear();
            if(!this->userCursors.isEmpty())
                this->userCursors.clear();
            if(userIDColor.isEmpty())
                userIDColor.clear();

            for(auto user : array_tmp) {
                QString username = user.toString();
                QColor random = QColor(rand()%255, rand()%255, 0);
                int userID = userIDs[i].toInt();
                int userPos = userCursors[i].toInt();
                userColor.insert(username, random);
                userIDColor.insert(userID, random);
                this->userCursors.insert(userID, userPos);
                i++;
                emit UserConnect(username, random);
            }
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
                QString font = v.toObject().value("font").toString();
                qDebug() << "font received = " << font;
                QFont f;
                f.fromString(font);
                format.setFont(f);

                QString colorName = v.toObject().value("color").toString();
                QColor color(colorName);
                format.setForeground(color);

                int align = v.toObject().value("align").toInt();
                Qt::AlignmentFlag alignFlag = static_cast<Qt::AlignmentFlag>(align);

                letters.append(std::move(new Letter(letter, fractionals, ID, format, alignFlag)));
                //qDebug() << "Lettera:" << letter;
            }
            json_buffer.clear();

            /*Creo il FileHandler*/
            connect( this->fileh, SIGNAL(localInsertNotify(QChar, QJsonArray, int, int, int, QTextCharFormat, Qt::AlignmentFlag)),
                     this, SLOT(sendInsert(QChar, QJsonArray, int, int, int, QTextCharFormat, Qt::AlignmentFlag)));
            connect( this->fileh, SIGNAL(localDeleteNotify(QString, int, int)), this, SLOT(sendDelete(QString, int, int)) );
            connect( this->fileh, SIGNAL(localStyleChangeNotify(QString, QString, int, QString, QString)),
                     this, SLOT(sendChangeStyle(QString, QString, int, QString, QString)));
            connect( this->fileh, SIGNAL(localCursorChangeNotify(int)),
                     this, SLOT(sendCursor(int)));
            connect( this->fileh, SIGNAL(localAlignChangeNotify(Qt::AlignmentFlag,int,QString,QString)),
                     this, SLOT(sendAlignment(Qt::AlignmentFlag,int,QString,QString)));

            /*Salvo il file come vettore di Letters nel fileHandler*/
            this->fileh->setValues(std::move(letters));
            // passare mappa<userid,pos>, mappa<userid,colore>
            emit readyFile(userCursors, userIDColor);
        }
    }
    else if(type.compare("ICON")==0){
        int remaining = object.value("remaining").toInt();
        QString chunk = object.value("chunk").toString();
        icon_buffer.append(chunk);
        if(remaining == 0){
            QImage img(2048,1024,QImage::Format_Indexed8);
            img = QImage::fromData(QByteArray::fromBase64(icon_buffer),"png");
//            QString imagePath(QStringLiteral("path/image.jpeg"));
//            if(img.save(QString::number(clientID),"png")){
//                qDebug() << "Icon saved";
//            }
//            else{

//                qDebug() << "Icon not save";
//            }
            QImageWriter writer(QString::number(clientID)+".png");
            writer.write(img);
            qDebug() << writer.error();
            icon_buffer.clear();
            emit iconThere();
        }
    }
    else if (type.compare("INSERT")==0) {
        QChar newLetterValue = object.value("letter").toString().at(0);
        QJsonArray position = object.value("position").toArray();
        int siteID = object.value("siteID").toInt();
        int siteCounter = object.value("siteCounter").toInt();
        int externalIndex = object.value("externalIndex").toInt();

        /* Estrarre stile lettera */
        QTextCharFormat format;
        QString font = object.value("font").toString();
        QFont f;
        f.fromString(font);
        format.setFont(f);

        QString colorName = object.value("color").toString();
        QColor color(colorName);
        format.setForeground(color);

        int align = object.value("align").toInt();
        Qt::AlignmentFlag alignFlag = static_cast<Qt::AlignmentFlag>(align);


        /*Inserire nel modello questa lettera e aggiornare la UI*/
        emit readyInsert(position, newLetterValue, externalIndex, siteID, siteCounter, format, alignFlag);
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
        QString filename = object.value("filename").toString();
        if(id != -1){
            this->fileh->setFileId(id);
            this->fileh->setSize(0);
            this->fileh->getVectorFile().clear();
            emit writeURI(object.value("URI").toString());
            /*Creo il FileHandler*/
            connect( this->fileh, SIGNAL(localInsertNotify(QChar, QJsonArray, int, int, int, QTextCharFormat, Qt::AlignmentFlag)),
                     this, SLOT(sendInsert(QChar, QJsonArray, int, int, int, QTextCharFormat, Qt::AlignmentFlag)) );
            connect( this->fileh, SIGNAL(localDeleteNotify(QString, int, int)), this, SLOT(sendDelete(QString, int, int)) );
            connect( this->fileh, SIGNAL(localStyleChangeNotify(QString, QString, int, QString, QString)),
                     this, SLOT(sendChangeStyle(QString, QString, int, QString, QString)));
            connect( this->fileh, SIGNAL(localCursorChangeNotify(int)),
                     this, SLOT(sendCursor(int)));
            connect( this->fileh, SIGNAL(localAlignChangeNotify(Qt::AlignmentFlag,int,QString,QString)),
                     this, SLOT(sendAlignment(Qt::AlignmentFlag,int,QString,QString)));
            qDebug() << "Il file è stato creato correttamente!";
            emit fileCreated(filename);
        }
        else{
            qDebug() << "Il File non è stato creato";
            emit fileCreated("null");
        }
    }

    else if(type.compare("STYLE")==0) {
        QString initialIndex = object.value("startIndex").toString();
        QString lastIndex = object.value("lastIndex").toString();
        QString changedStyle = object.value("changedStyle").toString();
        QString font = object.value("font").toString();
        emit readyStyleChange(initialIndex, lastIndex, changedStyle, font);
    }
    else if(type.compare("ALIGNMENT")==0) {
        /*
         * 1: left, 2: right, 132: center, 8: justify
        */
        int align = object.value("align").toInt();
        int cursor = object.value("cursor").toInt();
        Qt::AlignmentFlag alignFlag = static_cast<Qt::AlignmentFlag>(align);
        QString startID = object.value("startID").toString();
        QString lastID = object.value("lastID").toString();
        emit readyAlignChange(alignFlag, cursor, startID, lastID);
    }
    else if(type.compare("COLOR")==0) {
        QString startID = object.value("startID").toString();
        QString lastID = object.value("lastID").toString();
        QString colorName = object.value("color").toString();
        QColor color(colorName);
        emit colorChange(startID, lastID, color);
    }
    else if(type.compare("USER_CONNECT")==0){
        QString username = object.value("username").toString();
        int userID = object.value("userID").toInt();
        int cursor = object.value("cursor").toInt();
        QColor random = QColor(rand()%255, rand()%255, rand()%255, rand()%255);
        userColor.insert(username, random);
        userIDColor.insert(userID, random);
        userCursors.insert(userID, cursor);
        emit UserConnect(username, random);
        emit userCursor(qMakePair(userID,cursor), random);
    }
    else if(type.compare("USER_DISCONNECT")==0){
        QString username = object.value("username").toString();
        int userID = object.value("userID").toInt();
        userColor.remove(username);
        userIDColor.remove(userID);
        userCursors.remove(userID);
        emit userCursor(qMakePair(userID,-1), nullptr);
        emit UserDisconnect(username, userID);
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
    else if(type.compare("CURSOR")==0) {
        int userID = object.value("userID").toInt();
        if(userCursors.contains(userID) && userIDColor.contains(userID)) {
            // utente attivo sul file
            int position = object.value("position").toInt();
            userCursors[userID] = position;
            QColor color = userIDColor.value(userID);
            emit userCursor(qMakePair(userID, position), color);
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
   
    else if(type.compare("LOGIN")==0){
        checkLoginAndGetListFileName(object);
    }
    else if (type.compare("SIGNUP_RESPONSE")==0) {
        checkSignUp(object);
    }
    else if (type.compare("HISTORY")==0){
         getUsernames(object);
    }
    //if(socket->bytesAvailable())
      //  emit myReadyRead();
    qDebug() << "Finished!";
    emit socket->readyRead();
}


int Socket::sendInsert(QChar newLetterValue, QJsonArray position, int siteID, int siteCounter, int externalIndex, QTextCharFormat format, Qt::AlignmentFlag align)
{
    /*RICHIESTA*/
    QJsonObject obj;
    obj.insert("type", "INSERT");
    obj.insert("fileid", this->fileh->getFileId());
    obj.insert("letter", QJsonValue(newLetterValue));
    obj.insert("position", QJsonValue(position));
    obj.insert("font", QJsonValue(format.font().toString()));
    obj.insert("align", QJsonValue(align));
    obj.insert("color", QJsonValue(format.foreground().color().name()));
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

int Socket::sendChangeStyle(QString firstLetterID, QString lastLetterID, int fileID, QString changedStyle, QString font){
    /* Notificare il cambiamento di stile */
    //QString startID = letterFormatMap.firstKey();
    //QString endID = letterFormatMap.lastKey();

    QJsonObject obj;
    obj.insert("type", "STYLE");
    obj.insert("fileid", fileID);
    obj.insert("startIndex", firstLetterID);
    obj.insert("lastIndex", lastLetterID);
    //obj.insert("changedStyle", changedStyle);
    obj.insert("font", font);

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

int Socket::sendCursor(int position) {
    QJsonObject obj;
    obj.insert("type", "CURSOR");
    obj.insert("userID", clientID);
    obj.insert("position", position);
    obj.insert("fileid", fileh->getFileId());

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

int Socket::sendAlignment(Qt::AlignmentFlag alignment, int cursorPosition, QString startID, QString lastID) {
    QJsonObject obj;
    obj.insert("type", "ALIGNMENT");
    obj.insert("userID", clientID);
    obj.insert("align", alignment);
    obj.insert("cursor", cursorPosition);
    obj.insert("startID", startID);
    obj.insert("lastID", lastID);
    obj.insert("fileid", fileh->getFileId());

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

int Socket::sendColor(QString startID, QString lastID, QString color) {
    QJsonObject obj;
    obj.insert("type", "COLOR");
    obj.insert("userID", clientID);
    obj.insert("color", color);
    obj.insert("startID", startID);
    obj.insert("lastID", lastID);
    obj.insert("fileid", fileh->getFileId());

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
        emit noConnection();
}

void Socket::socketClosed()
{
        qDebug() << "Connection closed\n";

        emit noConnection();
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

void Socket::notConnected(){
    serverDisc *s = new serverDisc(this);
    hide();
    s->show();
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

bool Socket::isConnected(){
    return connected;
}

QString Socket::getClientUsername(){
    return this->username;
}

bool Socket::getConnection(){
    return this->connected;
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

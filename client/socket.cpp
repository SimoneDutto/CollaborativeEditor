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

    //connect( socket, SIGNAL(readyRead()),  SLOT(socketReadyReadFile()));
    connect( socket, SIGNAL(readyRead()),  SLOT(socketReadyReadListFiles()));

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

void Socket::socketReadyReadListFiles()
{
    //RICEZIONE LISTA FILES DAL SERVER

    qDebug() << "Inizio a leggere i file a cui ho accesso";
    QByteArray data = socket->readAll();
    QJsonDocument document = QJsonDocument::fromJson(data);
    QJsonObject object = document.object();

    QJsonValue value_id = object.value("id");
    clientID = value_id.toInt();
    if(clientID == -1){
        emit loginError();
        return;
    }
    QJsonValue value = object.value("files");
    QJsonArray nameFilesArray = value.toArray();

    qDebug() << "Accessed files:";
    foreach (const QJsonValue& v, nameFilesArray)
    {
        QString q = v.toObject().value("filename").toString();
        qDebug() << q;
        listFiles.append(q);
    }

    disconnect(socket, SIGNAL(readyRead()), this, SLOT(socketReadyReadListFiles()));
    connect( socket, SIGNAL(readyRead()),  SLOT(socketReadyReadFile()));

    emit loginSuccess();
    qDebug() << "Finished!";
    return;
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

int Socket::openFile(QString name_file)
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

void Socket::socketReadyReadFile()
{
    /*RICEZIONE FILE DAL SERVER*/

    qDebug() << "Inizio a leggere";

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
    QJsonValue value = object.value("letterArray");
    QJsonArray letterArray = value.toArray();

    foreach (const QJsonValue& v, letterArray)
    {
        Letter letter_tmp = Letter(v.toObject().value("value").toString(),
                 v.toObject().value("id").toString(),
                 v.toObject().value("pos_intera").toInt(),
                 v.toObject().value("pos_decimale").toInt());

        qDebug() << "Lettera:" << letter_tmp.getValue();
    }


    qDebug() << "Finished!";
    return;
}

inline qint32 ArrayToInt(QByteArray source)
{
    qint32 temp;
    QDataStream data(&source, QIODevice::ReadWrite);
    data >> temp;
    return temp;
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

#include "socket.h"
#include "ui_socket.h"
#include <QDebug>

Socket::Socket(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Socket)
{
    ui->setupUi(this);
}

Socket::Socket(const QString &host, quint16 port):blockSize(0)
{
    socket = new QTcpSocket(this);

    connect( socket, SIGNAL(connected()), SLOT(socketConnected()) );
    connect( socket, SIGNAL(disconnected()), SLOT(socketConnectionClosed()) );
    //connect( socket, SIGNAL(error(SocketError socketError)), SLOT(socketError(int)) );

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

void Socket::sendToServer()
    {
        // write to the server
        QTextStream os(socket);
        os << "Messaggio di prova\n";
    }

int Socket::openFile(QString name_file)
{
    /*RICHIESTA*/
    QJsonObject obj;
    obj.insert("type", "OPEN");
    obj.insert("filename:", name_file);

    if(socket->state() == QAbstractSocket::ConnectedState){
        qDebug() << "Richiesta:\n" << QJsonDocument(obj).toJson().data();
        socket->write(QJsonDocument(obj).toJson());
    }

    connect( socket, SIGNAL(readyRead()),  SLOT(socketReadyReadFile()));
    return socket->waitForBytesWritten();
}

void Socket::socketReadyReadFile()
{
    /*RICEZIONE FILE DAL SERVER*/
    disconnect(socket, SIGNAL(readyRead()), this, SLOT(socketReadyReadFile()));

    qDebug() << "Inizio a leggere";
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_12);

    /*Leggo dimensione file*/
    if(socket->bytesAvailable() < static_cast<qint64>(sizeof(quint32)))
        return; //!!!Controllare il cast se è corretto!!!
    in >> blockSize;
    qDebug() << blockSize;
    //if(socket->bytesAvailable() < blockSize) return -1;

    /*Ho già il nome, non so se il server lo manda, in caso negativo togliere queste righe*/
    QString fileName;
    in >> fileName;


    /*LETTURA FILE*/

    /*
    EXAMPLE JSON FILE
    {
        "letterArray": [{
            "value": "H",
            "id": "1-1",
            "pos_intera": 1,
            "pos_decimale": 0},
           {
            "value": "i",
            "id": "2-1",
            "pos_intera": 2,
            "pos_decimale": 0}
        ]
    }
    */

    QByteArray JsonFile = socket->readAll();
    QJsonDocument document = QJsonDocument::fromJson(JsonFile);
    QJsonObject object = document.object();
    QJsonValue value = object.value("letterArray");
    QJsonArray letterArray = value.toArray();


    foreach (const QJsonValue& v, letterArray)
    {
        Letter letter_tmp = Letter(v.toObject().value("value").toString(),
                 v.toObject().value("id").toString(),
                 v.toObject().value("pos_intera").toInt(),
                 v.toObject().value("pos_decimale").toInt());

        this->lastFilePtr->append(std::move(letter_tmp));

        qDebug() << letter_tmp.getValue();
    }

    qDebug() << "Finished!";
    return;
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

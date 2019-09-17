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
    connect( socket, SIGNAL(readyRead()),  SLOT(socketReadyRead()) );
    //connect( socket, SIGNAL(error(SocketError socketError)), SLOT(socketError(int)) );

    socket->connectToHost(host, port);

    /*if(socket->waitForConnected(3000))
    {
        //CONNESSO
        //socket->close();
        //socket->write("Prova");
    }
    else {
        //NON CONNESSO
    }*/
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

int Socket::openFile(QString name_file, QVector<Letter>& arrayFile)
{
    /*RICHIESTA*/
    QJsonObject obj;
    obj.insert("OPEN", name_file);

    if(socket->state() == QAbstractSocket::ConnectedState){
        socket->write(QJsonDocument(obj).toJson());
    }

    /*RICEZIONE*/
    qDebug() << "Inizio a leggere";
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_12);

    /*Leggo dimensione file*/
    if(blockSize == 0){
        if(socket->bytesAvailable() < static_cast<qint64>(sizeof(quint32))) return -1; //!!!Controllare il cast se è corretto!!!
        in >> blockSize;
    }
    if(socket->bytesAvailable() < blockSize) return -1;

    /*Ho già il nome, non so se il server lo manda, in caso negativo togliere queste righe*/
    QString fileName;
    in >>fileName;


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

    arrayFile = std::move(fileLikeLetterArray);

    qDebug() << "Finished!";
    return 1;
}

void Socket::socketReadyRead()
    {
        // read from the server
        qDebug() << "Leggo:\n";
        while ( socket->canReadLine() ) {
            qDebug() << socket->readLine();
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

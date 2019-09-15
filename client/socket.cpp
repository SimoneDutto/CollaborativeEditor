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
    connect( socket, SIGNAL(connectionClosed()), SLOT(socketConnectionClosed()) );
    connect( socket, SIGNAL(readyRead()),  SLOT(socketReadyRead()) );
    connect( socket, SIGNAL(error(int)), SLOT(socketError(int)) );

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

int Socket::openFile(QString name_file, QFile& file)
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

    /*Lettura file*/
    QByteArray line = socket->readAll();

    /*Cerco di creare il path*/
    QSettings settings;
    QString filePath = settings.value("Path/XmlTemporary", QVariant("")).toString(); //Mi ritorna sempre "" come il valore di default

    /*Considerare il caso in cui sia un path*/
    fileName = fileName.section("/", -1); //Divide per "/" e prende l'ultimo campo

    //QFile thisFile(filePath + "/" + fileName); nome inviato dal server


    /*!!!Provo ad usare un QFile passato per riferimento, in modo tale da salvarlo nel chiamante, non so se funziona!!!*/

    //QFile thisFile(filePath + "/" + name_file);
    file.setFileName(filePath + "/" + name_file);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Can't open file for written";
        return -1;
    }

    /*Salvo il file arrivato dal server*/
    file.write(line);
    file.seek(0); //Puntatore all'inizio del file

    file.close(); //Per ora CHIUDO, in realtà non dovrei perchè il file lo devo modificare
                  //una volta scaricato lo devo mostrare nell'applicazione

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

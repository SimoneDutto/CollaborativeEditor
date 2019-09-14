#include "socket.h"
#include "ui_socket.h"
#include <QDebug>

Socket::Socket(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Socket)
{
    ui->setupUi(this);
}

Socket::Socket(const QString &host, quint16 port)
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
        qDebug() << "Connesso!";
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

#ifndef SOCKET_H
#define SOCKET_H

#include <QDialog>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonValueRef>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSettings>
#include <QFile>
#include <QMap>
#include "letter.h"
#include "iostream"
#include "filehandler.h"

namespace Ui {
class Socket;
}

class Socket : public QDialog
{
    Q_OBJECT

public:
    explicit Socket(QWidget *parent = nullptr);
    Socket(const QString &host, quint16 port);
    ~Socket();
    FileHandler* getFHandler();
    int getClientID();
    QString getClientUsername();
    QMap<QString, int> getMapFiles();
    QMap<QString, QColor> getUserColor();
    bool isConnected();
    void isSigningUp(bool flag);
    bool getConnection();

private:
    Ui::Socket *ui;
    QTcpSocket *socket;
    QByteArray buffer;
    QByteArray json_buffer;
    QByteArray icon_buffer;
    long int size=0;
    FileHandler* fileh;
    int clientID;
    QString username;
    QMap<QString, int> mapFiles;
    QMap<QString, QColor> userColor;
    QMap<int, QColor> userIDColor;
    QMap<int, int> userCursors;

    QString pathIcon;
    bool isDoingSignUp;
    bool connected;

    void checkLoginAndGetListFileName(QJsonObject);
    void checkSignUp(QJsonObject);
    void sendIcon(QString path);
    void sendFileChunk(QByteArray chunk, QTcpSocket* socket, int remainingSize);

public slots:
    void sendSignUpRequest(QString username, QString password, QString pathImage);
    void sendLogin(QString username, QString password);
    void sendAccess(QString URI);

private slots:
    void closeConnection();
    void socketConnected();
    void socketConnectionClosed();
    void socketClosed();
    void socketError(int e);

    void notificationsHandler(QByteArray buffer);
    void readBuffer();
    //void setSignals();

    int sendOpenFile(QString name_file);
    int sendInsert(QChar newLetterValue, QJsonArray position, int siteID, int siteCounter, int externalIndex, QTextCharFormat format);
    int sendDelete(QString deletedLetterID, int fileID, int siteCounter);
    int sendCheckFileName(QString fileNameTmp);
    int sendNewFile(QString filename);
    int sendChangeStyle(QString firstLetterID, QString lastLetterID, int fileID, QString changedStyle, QString font);
    int sendCursor(int position);

signals:
    /* Registrazione */
    void signUpSuccess();
    void signUpError();
    void invalidUsername();

    /* Login */
    void loginSuccess();
    void loginError();

    /*Notifiche durante l'uso*/
    void readyFile(QMap<int,int>,QMap<int,QColor>);
    void readyInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter, QTextCharFormat format);
    void readyDelete(QString deletedLetterID);
    void readyStyleChange(QString firstID, QString lastID, QString changedStyle, QString font);
    void UserConnect(QString username, QColor colorUser);
    void userCursor(QPair<int,int> userID_pos, QColor color);
    void UserDisconnect(QString username, int userID);

    /*Signal connected to readyReady()*/
    void myReadyRead();
    void bufferReady(QByteArray data);

    void uriIsOk(QString uri);
    void uriIsNotOk();

    void writeURI(QString uri);

    void noConnection();
};



#endif // SOCKET_H

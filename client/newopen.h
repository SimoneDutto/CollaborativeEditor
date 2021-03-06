#ifndef NEWOPEN_H
#define NEWOPEN_H

#include <QDialog>
#include "socket.h"
#include "mainwindow.h"

namespace Ui {
class NewOpen;
}

class NewOpen : public QDialog
{
    Q_OBJECT

public:
    explicit NewOpen(Socket *sock = nullptr, FileHandler *fHandler = nullptr, QWidget *parent = nullptr);
    ~NewOpen();

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

    void uriIsOk(QString uri);
    void uriIsNotOk();
    void changeClientImage(QString path);
    void on_actionLog_Out_triggered();
    void on_discardImage_clicked();
    void setImage();
    //void createNewFile(QString filename);

private:
    Ui::NewOpen *ui;
    Socket *socket;
    MainWindow *mainwindow;
    FileHandler *fileHandler;
    Form *form;
    Uri *uri;
    Account *account;


signals:
    void openThisFile(QString fileName);
    void newFile(QString name);
    void checkUri(QString uri);
    void logOut();
};

#endif // NEWOPEN_H

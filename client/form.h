#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include "filehandler.h"
#include "socket.h"

namespace Ui {
class Form;
}

class Form : public QDialog
{
    Q_OBJECT

public:
    explicit Form(Socket *sock = nullptr, QWidget *parent = nullptr);
    ~Form();

private slots:
    void on_pushButton_clicked();

private:
    Ui::Form *ui;
    Socket *socket;

signals:
    void newFile(QString filename);
    void destroyMa(QString );

};

#endif // FORM_H

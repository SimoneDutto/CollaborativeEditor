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
    explicit Form(QWidget *parent = nullptr, Socket *sock = nullptr);
    ~Form();

private slots:
    void on_pushButton_clicked();

private:
    Ui::Form *ui;

signals:
    void newFile(QString filename);

};

#endif // FORM_H

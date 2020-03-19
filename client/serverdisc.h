#ifndef SERVERDISC_H
#define SERVERDISC_H

#include <QDialog>

namespace Ui {
class serverDisc;
}

class serverDisc : public QDialog
{
    Q_OBJECT

public:
    explicit serverDisc(QWidget *parent = nullptr);
    ~serverDisc();

private slots:
    void on_pushButton_clicked();

private:
    Ui::serverDisc *ui;

signals:
    void logOut();
};



#endif // SERVERDISC_H

#ifndef USERSLETTERSWINDOW_H
#define USERSLETTERSWINDOW_H

#include <QDialog>
#include <QColor>
#include <QMap>
#include <QListWidgetItem>
#include <QDebug>
#include "letter.h"

namespace Ui {
class usersLettersWindow;
}

class usersLettersWindow : public QDialog
{
    Q_OBJECT

public:
    explicit usersLettersWindow(QMap<int, QString> mapIdUsername, QVector<Letter*> letters, QWidget *parent = nullptr);
    ~usersLettersWindow();

private:
    Ui::usersLettersWindow *ui;
    QVector<Letter*> letters;
    QMap<QString, QColor> colorUser;
    QMap<int, QString> mapIdUsername;

};

#endif // USERSLETTERSWINDOW_H

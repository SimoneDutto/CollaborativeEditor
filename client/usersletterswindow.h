#ifndef USERSLETTERSWINDOW_H
#define USERSLETTERSWINDOW_H

#include <QDialog>
#include <QColor>
#include <QMap>
#include <QListWidgetItem>
#include "letter.h"

namespace Ui {
class usersLettersWindow;
}

class usersLettersWindow : public QDialog
{
    Q_OBJECT

public:
    explicit usersLettersWindow(QVector<Letter*> letters, QWidget *parent = nullptr);
    ~usersLettersWindow();

private:
    Ui::usersLettersWindow *ui;
    QVector<Letter*> letters;
    QMap<QString, QColor> colorUser;

};

#endif // USERSLETTERSWINDOW_H

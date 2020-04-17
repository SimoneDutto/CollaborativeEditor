#include "usersletterswindow.h"
#include "ui_usersletterswindow.h"

usersLettersWindow::usersLettersWindow(QVector<Letter*> letters, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::usersLettersWindow),
    letters(letters)
{
    /* Setto fissa la dimensione della finestra */
    ui->setupUi(this);
    this->setFixedHeight(620);
    this->setFixedWidth(900);


    /*Aggiornare la GUI con il testo*/
    QTextCursor cursor(ui->testo->textCursor());
    cursor.setPosition(0);

    /****************/
    QList<QString> listTEST = {"Marco", "Luca", "Gio", "Lucia", "Alessia"};
    int i = 0;
    /****************/

    for(Letter *l : letters){
        //QMap<QString, QColor>::iterator ite = colorUser.find(l->getUsername());

        /****************/
        i++;
        QMap<QString, QColor>::iterator ite = colorUser.find(listTEST.at(i%5));

        /****************/

        if(ite != colorUser.end()){
            //Già esiste la coppia User-Color e aggiungo la lettere con sfondo il colore
            auto format = l->getFormat();
            format.setBackground(QBrush(ite.value(), Qt::SolidPattern));
            cursor.insertText(l->getValue(), format);
        }

        else {
            //Creo la coppia User-Color nella mappa
            QColor* color = new QColor(rand()%255, rand()%255, rand()%255, rand()%255);
            QColor newColor = color->lighter(120);
            newColor.setAlpha(150);
            //colorUser.insert(l->getUsername(), newColor);

            /****************/
            colorUser.insert(listTEST.at(i%5), newColor);
            /****************/

            //Aggiungo lo User nella ListUsers
            //QListWidgetItem *user = new QListWidgetItem(l->getUsername());

            /****************/
            QListWidgetItem *user = new QListWidgetItem(listTEST.at(i%5));
            /****************/

            newColor.setAlpha(255);
            newColor.lighter(255);
            user->setForeground(newColor);
            QFont font( "", 15, QFont::Bold);
            user->setFont(font);
            user->setFlags(Qt::ItemIsEnabled);
            ui->listUsers->addItem(user);

            //Aggiungo la lettere con sfondo il colore
            newColor.setAlpha(150);
            newColor.lighter(120);
            auto format = l->getFormat();
            format.setBackground(QBrush(newColor, Qt::SolidPattern));
            cursor.insertText(l->getValue(), format);
        }

    }

}

usersLettersWindow::~usersLettersWindow()
{
    delete ui;
}

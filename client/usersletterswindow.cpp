#include "usersletterswindow.h"
#include "ui_usersletterswindow.h"

usersLettersWindow::usersLettersWindow(QMap<int, QString> mapIdUsername, QVector<Letter*> letters, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::usersLettersWindow),
    letters(letters),
    mapIdUsername(mapIdUsername)
{
    /* Setto fissa la dimensione della finestra */
    ui->setupUi(this);
    this->setFixedHeight(620);
    this->setFixedWidth(900);
    this->setWindowTitle("");
    QPalette pal = palette();
    pal.setColor(QPalette::Background, QColor(58,58,60));
    pal.setColor(QPalette::WindowText, Qt::white);
    pal.setColor(QPalette::ButtonText, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->show();


    /*Aggiornare la GUI con il testo*/
    QTextCursor cursor(ui->testo->textCursor());
    cursor.setPosition(0);

    for(Letter *l : letters){
        QMap<int, QString>::iterator usernameIte = mapIdUsername.find(l->getUserId());
        if(usernameIte == mapIdUsername.end()){
            qDebug() << "Nella mapIdUsername non ho trovato l'id desiderato!!!";
        }
        QString username = usernameIte.value();

        QMap<QString, QColor>::iterator ite = colorUser.find(username);

        if(ite != colorUser.end()){
            //Già esiste la coppia User-Color e aggiungo la lettere con sfondo il colore
            auto format = l->getFormat();
            format.setBackground(QBrush(ite.value(), Qt::SolidPattern));
            cursor.insertText(l->getValue(), format);
            auto alignment = l->getAlignment();
            QTextBlockFormat blockFormat = cursor.blockFormat();
            blockFormat.setAlignment(alignment);
            cursor.mergeBlockFormat(blockFormat);
        }

        else {
            //Creo la coppia User-Color nella mappa
            QColor* color = new QColor(rand()%255, rand()%255, rand()%255, rand()%255);
            QColor newColor = color->lighter(120);
            newColor.setAlpha(150);
            colorUser.insert(username, newColor);

            //Aggiungo lo User nella ListUsers
            QListWidgetItem *user = new QListWidgetItem(username);

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
            auto alignment = l->getAlignment();
            QTextBlockFormat blockFormat = cursor.blockFormat();
            blockFormat.setAlignment(alignment);
            cursor.mergeBlockFormat(blockFormat);
        }

    }

}

usersLettersWindow::~usersLettersWindow()
{
    delete ui;
}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QFont>
#include <QFontDialog>
#include <QColor>
#include <QColorDialog>
#include <QTextCharFormat>
#include <QProcess>
#include <QByteArray>
#include <QPdfWriter>
#include <QPrinter>
#include <QStandardItem>
#include "serverdisc.h"
#include <QDesktopWidget>
#include <algorithm>


MainWindow::MainWindow(Socket *sock, FileHandler *fileHand,QWidget *parent, QString nome) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    socket(sock),
    fHandler(fileHand)
{
    ui->setupUi(this);
    QPalette pal = palette();

    // set black background
    pal.setColor(QPalette::Background, QColor(58,58,60));
    pal.setColor(QPalette::WindowText, Qt::white);
    pal.setColor(QPalette::ButtonText, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->show();

    QPalette p = ui->textEdit->palette(); // define pallete for textEdit..
    p.setColor(QPalette::Base, QColor(209,209,214)); // set color "Red" for textedit base
    p.setColor(QPalette::Text, Qt::black); // set text color which is selected from color pallete
    ui->textEdit->setPalette(p);

    // set picture
    /*QPixmap pix("path -- TO DO");
    ui->user1->setPixmap(pix);*/


    setWindowTitle(nome);
    ui->label_2->setStyleSheet("background-color:lightgray; color:black");
    ui->label_2->setTextInteractionFlags(Qt::TextSelectableByMouse);
    //ui->label->setStyleSheet("background-color:lightgray");

    auto availableSize = qApp->desktop();
       int width = availableSize->width();
       int height = availableSize->height();
       width *= 0.8; // 80% of the screen size
       height *= 0.8; // 80% of the screen size
       QSize newSize( width, height );

       setGeometry(
           QStyle::alignedRect(
               Qt::LeftToRight,
               Qt::AlignCenter,
               newSize,
               qApp->desktop()->rect()
           )
       );
    //ui->lineEdit->setText(nome);

    /* Personalizzo e aggiungo le label degli utenti connessi */
    QString styleSheet = QString("QGroupBox {border: 0px;}");
    ui->groupBox->setStyleSheet(styleSheet);

    ui->user1->hide();
    ui->user2->hide();
    ui->user3->hide();
    ui->counter->hide();

    /* Aggiungo nome e icona dell'utente */

    QString username = socket->getClientUsername();

    styleSheet = "QLabel { background-color: rgb(255, 252, 247); color: black; border-style: solid; border-width: 1px; border-radius: 3px; border-color: black; font: ; }";
    ui->username->setStyleSheet(styleSheet);
    QFont font("Arial");
    ui->username->setFont(font);
    ui->username->setText(username);

    QString imageName = QString::number(socket->getClientID())+".png";
    QPixmap userPixmap = QPixmap(imageName);

    if(userPixmap != QPixmap()){
        QPixmap scaled = userPixmap.scaled(ui->myicon->width(), ui->myicon->height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        ui->myicon->setPixmap(scaled);
    }

    else {
        styleSheet = "QLabel { background-color: rgb(255, 252, 247); color: black; border-style: solid; border-width: 2px; border-radius: 6px; border-color: orange; font: ; }";
        ui->myicon->setStyleSheet(styleSheet);
        QFont font("Arial", 30);
        ui->myicon->setFont(font);
        ui->myicon->setText(username.at(0).toUpper());
    }


    /* CONNECT per segnali uscenti, inoltrare le modifiche fatte */
    connect( this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat)),
              fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat)));
    connect( this, SIGNAL(myDelete(int,int)),
              fHandler, SLOT(localDelete(int,int)));
    connect( this, SIGNAL(sendNameFile(QString)),
              socket, SLOT(sendCheckFileName(QString)));
    connect( this, SIGNAL(newFile(QString)),
             socket, SLOT(sendNewFile(QString)));

    /* CONNECT per segnali entranti, applicare sulla GUI le modifiche che arrivano sul socket */

    connect( socket, SIGNAL(readyInsert(QJsonArray, QChar, int, int, int, QTextCharFormat)),
              fHandler,  SLOT(remoteInsert(QJsonArray, QChar, int, int, int, QTextCharFormat)));
    connect( socket, SIGNAL(readyDelete(QString)),
              fHandler, SLOT(remoteDelete(QString)));
    connect( socket, SIGNAL(readyFile(QMap<int,int>,QMap<int,QColor>)),  this, SLOT(fileIsHere(QMap<int,int>,QMap<int,QColor>)));
    connect( fHandler, SIGNAL(readyRemoteInsert(QChar, int, QTextCharFormat)),
             this, SLOT(changeViewAfterInsert(QChar, int, QTextCharFormat)));
    connect( fHandler, SIGNAL(readyRemoteDelete(int)),
             this, SLOT(changeViewAfterDelete(int)));
    connect( fHandler, SIGNAL(readyRemoteStyleChange(QString, QString)),
             this, SLOT(changeViewAfterStyle(QString, QString)));
    connect( socket, SIGNAL(readyStyleChange(QString, QString, QString, QString)),
             fHandler, SLOT(remoteStyleChange(QString, QString, QString, QString)));
    connect( socket, SIGNAL(UserConnect(QString, QColor)),
             this, SLOT(addUserConnection(QString, QColor)));
    connect( socket, SIGNAL(UserDisconnect(QString,int)),
             this, SLOT(removeUserDisconnect(QString,int)));
    connect( socket, SIGNAL(writeURI(QString)),
             this, SLOT(on_write_uri(QString)));

    /* CONNECT per lo stile dei caratteri */
    connect( this, SIGNAL(styleChange(QMap<QString, QTextCharFormat>, QString, QString, bool, bool, bool, QString)),
              fHandler, SLOT(localStyleChange(QMap<QString, QTextCharFormat>, QString, QString, bool, bool, bool, QString)) );

    /* CONNECT per cursore */
    connect( socket, SIGNAL(userCursor(QPair<int,int>,QColor)),
             this, SLOT(on_cursor_triggered(QPair<int,int>,QColor)));
    connect( this, SIGNAL(sendCursorChange(int)),
             fHandler, SLOT(localCursorChange(int)));

    /* CONNECT per collegare le ClickableLabel */
    connect( ui->user1, SIGNAL(clicked()),
             this, SLOT(on_counter_clicked()));
    connect( ui->user2, SIGNAL(clicked()),
             this, SLOT(on_counter_clicked()));
    connect( ui->user3, SIGNAL(clicked()),
             this, SLOT(on_counter_clicked()));
    connect( ui->myicon, SIGNAL(clicked()),
             this, SLOT(on_actionEdit_Profile_triggered()));


}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::sorting(QPair<QPair<int,QColor>,int> &e1, QPair<QPair<int,QColor>,int> &e2){
if(e1.second < e2.second)
return true;
else return false;
}

void MainWindow::on_actionNew_triggered()
{
    if(ui->textEdit->toPlainText().size() > 0){
        ui->textEdit->clear();
    }

    form = new Form(this->socket, this);
    form->show();
    //ui->lineEdit->setText(filename);
}

void MainWindow::on_actionOpen_triggered()
{
    //QString file_name = QFileDialog::getOpenFileName(this,"Open the file");
    dialog = new Dialog(this->socket, this);
    dialog->show();
//    QFile file(file_name);
//    file_path = file_name;
//    if(!file.open(QFile::ReadOnly | QFile::Text)){
//        QMessageBox::warning(this, "..", "file not open");
//        return;
//    }
//    QTextStream in(&file);
//    QString text = in.readAll();
//    ui->textEdit->setText(text);
//    file.close();
}

//void MainWindow::on_actionSave_triggered()
//{
//    if(file_path == ""){
//        on_actionSave_As_triggered();
//        return;
//    }else{
//    QFile file(file_path);
//    if(!file.open(QFile::WriteOnly | QFile::Text)){
//        QMessageBox::warning(this, "..", "file not saved!");
//        return;
//    }
//    QTextStream out(&file);
//    QString text = ui->textEdit->toPlainText();
//    out << text;
//   file.flush();
//    file.close();
//    }
//}

//void MainWindow::on_actionSave_As_triggered()
//{
//    QString file_name = QFileDialog::getSaveFileName(this,"Save the file");
//    QFile file(file_name);
//    file_path = file_name;
//    if(!file.open(QFile::WriteOnly | QFile::Text)){
//        QMessageBox::warning(this, "..", "file not saved!");
//        return;
//    }
//    QTextStream out(&file);
//    QString text = ui->textEdit->toPlainText();
//    out << text;
//    file.flush();
//    file.close();
//}

void MainWindow::on_actionCut_triggered()
{
    ui->textEdit->cut();
}

void MainWindow::on_actionCopy_triggered()
{
    ui->textEdit->copy();
}

void MainWindow::on_actionRedo_triggered()
{
    ui->textEdit->redo();
}

void MainWindow::on_actionPaste_triggered()
{
    ui->textEdit->paste();
}

void MainWindow::on_actionUndo_triggered()
{
    ui->textEdit->undo();
}

void MainWindow::on_actionAbout_us_triggered()
{
    QString about_text;
      about_text  = "Authors: Isabella Romita, Debora Caldarola, Vito Tassielli, Simone Dutto\n";
      about_text += "Date: 11/09/2019\n";
      about_text += "(C) Notepad  (R)\n";

      QMessageBox::about(this,"About Notepad",about_text);
}

void MainWindow::on_actionBold_triggered()
{
    //Conteggio inizia da 0, quindi num_tot = selectionEnd-selectionStart
    //Pensare come cursore non come lettere selezioante
    //Es. ciao0 ciao1 ciao2
    //Se seleziono ciao0
    //selectionStart = 0
    //selectionEnd = 5

    auto cursor = ui->textEdit->textCursor();
    qDebug() << "Selection start: " << cursor.selectionStart() << " end: " << cursor.selectionEnd();

    /*CASO1: Non sto selezionando niente, attivo/disattivo il grassetto*/
    if(cursor.selectionStart() - cursor.selectionEnd() == 0){
        if(ui->textEdit->fontWeight()!=75)
            ui->textEdit->setFontWeight(75);
        else
            ui->textEdit->setFontWeight(50);
    }

    /*CASO2: Cambio il grassetto di una selezione*/
    else{
        disconnect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat)));
        disconnect(this, SIGNAL(myDelete(int,int)),
                  fHandler, SLOT(localDelete(int,int)));

        qDebug() << "Seleziono un testo per grassetto";

        if(ui->textEdit->fontWeight()==50)
            ui->textEdit->setFontWeight(75);
        else
            ui->textEdit->setFontWeight(50);

        /* Aggiorno il modello */


        QMap<QString, QTextCharFormat> formatCharMap;
        auto vettore = this->fHandler->getVectorFile();
        int i=0;

        int start = cursor.selectionStart();
        int end = cursor.selectionEnd()-1;

        QString startID = vettore.at(start)->getLetterID();
        QString lastID = vettore.at(end)->getLetterID();

        for(i=start; i<=end; i++){
            cursor.setPosition(i+1);
            auto letterFormat = cursor.charFormat();
            qDebug() << letterFormat.fontWeight() << "---" << letterFormat.fontUnderline() << "---" << letterFormat.fontItalic();
            //vettore.at(i)->setFormat(letterFormat);
            qDebug() << "LetterID = " << vettore.at(i)->getLetterID();
            formatCharMap.insert(vettore.at(i)->getLetterID(), letterFormat);
        }

        emit styleChange(formatCharMap, startID, lastID, true, false, false, "none");

        connect( this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat)));
        connect( this, SIGNAL(myDelete(int,int)),
                  fHandler, SLOT(localDelete(int,int)));
    }
}

void MainWindow::on_actionItalic_triggered()
{
    auto cursor = ui->textEdit->textCursor();
    qDebug() << "Selection start: " << cursor.selectionStart() << " end: " << cursor.selectionEnd();

    /*CASO1: Non sto selezionando niente, attivo/disattivo il corsivo*/
    if(cursor.selectionStart() - cursor.selectionEnd() == 0){
        if(ui->textEdit->fontItalic()!=true)
            ui->textEdit->setFontItalic(true);
        else
            ui->textEdit->setFontItalic(false);
    }

    /*CASO2: Cambio il corsivo di una selezione*/
    else{
        disconnect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat)));
        disconnect(this, SIGNAL(myDelete(int,int)),
                  fHandler, SLOT(localDelete(int,int)));

        qDebug() << "Seleziono un testo per corsivo";
        qDebug() << ui->textEdit->fontItalic();
        //bool italic;
        //if(cursor.charFormat().fontItalic()==false)
        if(ui->textEdit->fontItalic()==false)
            ui->textEdit->setFontItalic(true);
            //italic = true;
        else
            ui->textEdit->setFontItalic(false);
            //italic = false;

        /* Aggiorno il modello */
        QMap<QString, QTextCharFormat> formatCharMap;
        auto vettore = this->fHandler->getVectorFile();
        int i=0;

        int start = cursor.selectionStart();
        int end = cursor.selectionEnd()-1;

        QString startID = vettore.at(start)->getLetterID();
        QString lastID = vettore.at(end)->getLetterID();

        for(i=start; i<=end; i++){
            cursor.setPosition(i+1);
            auto letterFormat = cursor.charFormat();
            /*if(italic)
                letterFormat.setFontItalic(true);
            else letterFormat.setFontItalic(false);
            cursor.setCharFormat(letterFormat);*/
            qDebug() << letterFormat.fontWeight() << "---" << letterFormat.fontUnderline() << "---" << letterFormat.fontItalic();
            //vettore.at(i)->setFormat(letterFormat);
            qDebug() << "LetterID = " << vettore.at(i)->getLetterID();

            formatCharMap.insert(vettore.at(i)->getLetterID(), letterFormat);
        }

        emit styleChange(formatCharMap, startID, lastID, false, true, false, "none");

        connect( this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat)));
        connect( this, SIGNAL(myDelete(int,int)),
                  fHandler, SLOT(localDelete(int,int)));
    }
}

void MainWindow::on_actionUnderlined_triggered()
{
    auto cursor = ui->textEdit->textCursor();
    qDebug() << "Selection start: " << cursor.selectionStart() << " end: " << cursor.selectionEnd();

    /*CASO1: Non sto selezionando niente, attivo/disattivo il sottolineato*/
    if(cursor.selectionStart() - cursor.selectionEnd() == 0){
        if(ui->textEdit->fontUnderline()!=true)
            ui->textEdit->setFontUnderline(true);
        else
            ui->textEdit->setFontUnderline(false);
    }

    /*CASO2: Cambio il sottolineato di una selezione*/
    else{
        disconnect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat)));
        disconnect(this, SIGNAL(myDelete(int,int)),
                  fHandler, SLOT(localDelete(int,int)));

        qDebug() << "Seleziono un testo per sottolineato";

        if(ui->textEdit->fontUnderline()!=true)
            ui->textEdit->setFontUnderline(true);
        else
            ui->textEdit->setFontUnderline(false);

        /* Aggiorno il modello */
        QMap<QString, QTextCharFormat> formatCharMap;
        auto vettore = this->fHandler->getVectorFile();
        int i=0;

        int start = cursor.selectionStart();
        int end = cursor.selectionEnd()-1;
        qDebug() << end;

        QString startID = vettore.at(start)->getLetterID();
        QString lastID = vettore.at(end)->getLetterID();

        for(i=start; i<=end; i++){
            /* Se testo selezionato misto, allora settare, altrimenti se tutto settato, togliere */
            cursor.setPosition(i+1);
            auto letterFormat = cursor.charFormat();
            qDebug() << letterFormat.fontWeight() << "---" << letterFormat.fontUnderline() << "---" << letterFormat.fontItalic();
            //vettore.at(i)->setFormat(letterFormat);
            formatCharMap.insert(vettore.at(i)->getLetterID(), letterFormat);
        }

        emit styleChange(formatCharMap, startID, lastID, false, false, true, "none");

        connect( this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat)));
        connect( this, SIGNAL(myDelete(int,int)),
                  fHandler, SLOT(localDelete(int,int)));
    }
}

void MainWindow::on_actionFont_triggered()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, this);
    if(ok)
        ui->textEdit->setFont(font);
    else
        return;
}

void MainWindow::on_actionColor_triggered()
{
    QColor color = QColorDialog::getColor(Qt::white,this,"Choose a color");
    if(color.isValid())
        ui->textEdit->setTextColor(color);
}

void MainWindow::on_actionBackgorund_Color_triggered()
{
    QColor color = QColorDialog::getColor(Qt::white,this,"Choose a color");
    if(color.isValid())
        ui->textEdit->setTextBackgroundColor(color);
}

void MainWindow::on_textEdit_textChanged()
{
    /*Testo cambiato con INSERT */
    QTextCursor cursor(ui->textEdit->textCursor());
    int externalIndex = cursor.position();
    int numberOfLetters = ui->textEdit->toPlainText().size();

    //ui->statusBar->showMessage(QString::number(pos));
    /*qDebug() << "External index = " << externalIndex;
    qDebug() << "Letter cnt prev = " << letterCounter;
    qDebug() << "Letter cnt post = " << numberOfLetters;*/

    if(numberOfLetters >= letterCounter) {   // Compare actual number of letters in editor to the previous situation
        QChar newLetterValue = ui->textEdit->toPlainText().at(externalIndex-1);
        qDebug() << "!!!!!!!!!!!!!!!!!!!!!insert";
        letterCounter++;
        //ui->statusBar->showMessage(c);
        emit myInsert(externalIndex, newLetterValue, socket->getClientID(), cursor.charFormat());
        emit sendCursorChange(externalIndex);
    }
    else if (numberOfLetters < letterCounter){  /*Testo cambiato con DELETE */
        /*disconnect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
        // undo last operation to retrieve deleted chars (necessary to handle simultaneous deleting)
        ui->textEdit->undo();
        qDebug() << ui->textEdit->toPlainText();
        int undoSize = ui->textEdit->toPlainText().size();
        ui->textEdit->redo();
        int redoSize = ui->textEdit->toPlainText().size();
        qDebug() << ui->textEdit->toPlainText();
        connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));*/

        // lettere consecutive => basta trovare la differenza delle dimensioni
        int deletedLetters = letterCounter - numberOfLetters;

        qDebug() << "!!!!!!!!!!!!!!!!!!!!!delete";
        letterCounter -= deletedLetters;
        // check: selection start 0 crasha. Selezione/deselezione più volte
        emit myDelete(externalIndex+1, externalIndex+deletedLetters);
    }
}



void MainWindow::on_lineEdit_editingFinished()
{
    /*Cambio il nome del documento, solo dopo l'OK*/
    //emit sendNameFile(ui->lineEdit->text());
}

void MainWindow::fileIsHere(QMap<int,int> id_pos, QMap<int,QColor> id_colore){
    qDebug() << "FileIsHere";
    disconnect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));

    /*Aggiornare la GUI con il file appena arrivato*/
    QTextCursor cursor(ui->textEdit->textCursor());
    cursor.setPosition(0);
    auto vettore = this->fHandler->getVectorFile();

    for(auto lettera : vettore){
        cursor.insertText(lettera->getValue(), lettera->getFormat());
    }
    letterCounter = ui->textEdit->toPlainText().size();
    qDebug() << "letter cnt : = "<< letterCounter;

    /* cursore */

    /*simulo le mappe che dovrebbero arrivarmi */

//    QMap<int,int> id_pos;
//    QMap<int, QColor> id_colore;

//    id_pos.insert(1,6);
//    id_pos.insert(5,3);
//    id_pos.insert(3,12);

//    id_colore.insert(1, Qt::white);
//    id_colore.insert(5, Qt::red);
//    id_colore.insert(3, Qt::blue);

    /* la parte sopra andrà cancellata */

    // riempio la lista indicizzata

    for(int utente : id_pos.keys()){
        id_colore_cursore.append(qMakePair(qMakePair(utente,id_colore.value(utente)),id_pos.value(utente)));
    }

    std::sort(id_colore_cursore.begin(), id_colore_cursore.end(), sorting);

    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
}

void MainWindow::changeViewAfterInsert(QChar l, int pos, QTextCharFormat format)
{
    disconnect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));

    QTextCursor cursor(ui->textEdit->textCursor());
    cursor.setPosition(pos);
    cursor.insertText(l, format);
    letterCounter++;

    //CONTROLLO SE ARRIVA IL FORMATO GIUSTO
    /*qDebug() << "Lettera che sto inserendo: " << l;
    qDebug() << "Grassetto" << format.fontWeight();
    qDebug() << "Sottolineato" << format.fontUnderline();
    qDebug() << "Corsivo" << format.fontItalic();*/

    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
}

void MainWindow::changeViewAfterDelete(int pos)
{
    disconnect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));

    QTextCursor cursor(ui->textEdit->textCursor());
    cursor.setPosition(pos);
    cursor.deletePreviousChar();
    qDebug() << "Devo cancellare la lettera in pos: " << pos;

    letterCounter--;
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
}


void MainWindow::changeViewAfterStyle(QString firstID, QString lastID) {
    disconnect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
    auto cursor = ui->textEdit->textCursor();
    bool intervalStarted = false;

    QVector<Letter*> vectorFile = this->fHandler->getVectorFile();
    QString text = "";
    int count = 0;
    for(Letter *l : vectorFile){
        count ++;

        if(l->getLetterID() == firstID) intervalStarted=true;

        if(intervalStarted){
            cursor.setPosition(count);
            cursor.deletePreviousChar();
            cursor.insertText(l->getValue(), l->getFormat());

            //CONTROLLO SE ARRIVA IL FORMATO GIUSTO
            /*qDebug() << "Lettera cambio stile: " << l->getValue();
            qDebug() << "Grassetto" << l->getFormat().fontWeight();
            qDebug() << "Sottolineato" << l->getFormat().fontUnderline();
            qDebug() << "Corsivo" << l->getFormat().fontItalic();*/
        }

        if(l->getLetterID() == lastID) break;
    }
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));

}

void MainWindow::addUserConnection(QString username, QColor color){

    int numberUsersOnline = socket->getUserColor().size();
    QString styleSheet = "QLabel { background-color: rgb(255, 252, 247); color: black; border-style: solid; border-width: 3px; border-radius: 15px; border-color: %1; font: ; }";

    if(numberUsersOnline == 1){  //Personalizzo ed accendo la label user1
        ui->user1->setStyleSheet(styleSheet.arg(color.name()));
        ui->user1->setText(username.at(0).toUpper());
        ui->user1->show();
    }

    else if(numberUsersOnline == 2){  //Personalizzo ed accendo la label user2
        ui->user2->setStyleSheet(styleSheet.arg(color.name()));
        ui->user2->setText(username.at(0).toUpper());
        ui->user2->show();
    }

    else if(numberUsersOnline == 3){  //Personalizzo ed accendo la label user3
        ui->user2->setStyleSheet(styleSheet.arg(color.name()));
        ui->user2->setText(username.at(0).toUpper());
        ui->user2->show();
    }

    else {  //Incrementare il contatore
        ui->counter->setText("+" + QString::number(numberUsersOnline));
        ui->counter->show();
    }

    /*La lista completa degli Online Users la inizializzo nel OnlineUser Constructor*/

}

void MainWindow::removeUserDisconnect(QString, int userID){


    int numberUsersOnline = socket->getUserColor().size();

    if(numberUsersOnline == 0){  //Spengo la label user1
        ui->user1->hide();
    }

    else if(numberUsersOnline == 1){  //Spengo la label user2
        ui->user2->hide();
    }

    else if(numberUsersOnline == 2){  //Spengo la label user3
        ui->user3->hide();
    }

    else {
        ui->counter->hide();
    }

    for(int i = 0; i< id_colore_cursore.size(); i++){
        //se c'è lo sostituisco
        if (id_colore_cursore.at(i).first.first == userID ){
            id_colore_cursore.removeAt(i);
        }
    }
    /*La lista completa degli Online Users la inizializzo nel OnlineUser Constructor*/

}

//TODO: inserire gestione bottoni
void MainWindow::on_textEdit_cursorPositionChanged() {

    /*Questa funzione gestirà la vista dei bottoni dello stile, ovvero se si vedrenno accessi o spenti. */

    QTextCursor cursor(ui->textEdit->textCursor());
    int pos = cursor.position();
    // emit segnale per notificare altri utenti del cambiamento
    if(pos <= ui->textEdit->toPlainText().size())
        emit sendCursorChange(pos);

    /*Se il testo selezionato ha stile misto, i bottoni accendono lo stile*/
    if(cursor.hasSelection()==true){
        if(ui->textEdit->fontWeight()!=75){
            ui->actionBold->setChecked(false);
        }
        else {
            ui->actionBold->setChecked(true);
        }

        if(ui->textEdit->fontItalic()!=true){
            ui->actionItalic->setChecked(false);
        }
        else {
            ui->actionItalic->setChecked(true);
        }

        if(ui->textEdit->fontUnderline()!=true){
            ui->actionUnderlined->setChecked(false);
        }
        else {
            ui->actionUnderlined->setChecked(true);
        }

    }


    /*Catturiamo lo stile del carattere precedente per settare i bottoni ON/OFF*/
    else {
        auto format = cursor.charFormat();

        if(format.fontWeight()!=75){
            ui->actionBold->setChecked(false);
        }
        else {
            ui->actionBold->setChecked(true);
        }

        if(format.fontItalic()==false){
            ui->actionItalic->setChecked(false);
        }
        else {
            ui->actionItalic->setChecked(true);
        }

        if(format.fontUnderline()==false){
            ui->actionUnderlined->setChecked(false);
        }
        else {
            ui->actionUnderlined->setChecked(true);
        }
    }

}

void MainWindow::on_actionLog_Out_triggered()
{
    emit logOut();
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}

void MainWindow::on_actionEdit_Profile_triggered()
{
    account = new Account(this->socket, this, this->windowTitle());
    account->show();

}

/*void MainWindow::on_actionGet_URI_triggered()
{
    // ricavare URI da passare al costruttore
    uri = new Uri(socket,this,"QUI USCIRA' L'URI");
    uri->show();
}*/



void MainWindow::on_actionExport_as_PDF_triggered()
{
    QTextDocument document;
    document.setPlainText(ui->textEdit->toPlainText());

    QString fn = QFileDialog::getSaveFileName(this, tr("Select output file"), QString(), tr("PDF Files(*.pdf)"));
      if (fn.isEmpty())
        return;
    QPrinter printer;
    printer.setPageMargins(10.0,10.0,10.0,10.0,printer.Millimeter);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setColorMode(QPrinter::Color);
    printer.setOutputFileName(fn);
    document.print(&printer);
    //emit exportAsPDF();
}

void MainWindow::on_counter_clicked()
{
    OnlineUser *onlineList = new OnlineUser(socket, this);
    onlineList->show();
}

void MainWindow::on_write_uri(QString uri){
    ui->label_2->setText(uri);
}

void MainWindow::on_actionAlign_to_Left_triggered()
{
    ui->textEdit->setAlignment(Qt::AlignLeft);
}

void MainWindow::on_actionAlign_to_Right_triggered()
{
    ui->textEdit->setAlignment(Qt::AlignRight);
}

void MainWindow::on_actionAlign_to_Center_triggered()
{
    ui->textEdit->setAlignment(Qt::AlignCenter);
}

void MainWindow::on_actionAlign_to_Justify_triggered()
{
    ui->textEdit->setAlignment(Qt::AlignJustify);
}

void MainWindow::notConnected(){
    serverDisc *s = new serverDisc(this);
    s->show();
}


void MainWindow::on_cursor_triggered(QPair<int,int> idpos, QColor col)
{
    // controllo che nella mappa colorecursore non sia gia presente l'id

    disconnect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
    QTextCharFormat fmt;
    QTextCharFormat fmt2;

    fmt2.setBackground(QColor(209,209,214));
    QTextCursor cursor = ui->textEdit->textCursor();

    // simulo la coppia userid-pos e il colore che mi arriveranno come parametri
    //QPair<int, int> idpos = qMakePair(5, 11);
    //QColor col = Qt::red;

    // controllo che nella mappa colorecursore non sia gia presente il colore
    bool trovato = false;
    for(int i = 0; i< id_colore_cursore.size(); i++){
        //se c'è lo sostituisco
        if (id_colore_cursore.at(i).first.first == idpos.first ){
            id_colore_cursore.replace(i, qMakePair(qMakePair(idpos.first,col), idpos.second));
            trovato = true;
            break;
        }
    }
    //altrimenti lo aggiungo
    if (trovato == false)
        id_colore_cursore.append(qMakePair(qMakePair(idpos.first,col), idpos.second));

    std::sort(id_colore_cursore.begin(), id_colore_cursore.end(), sorting);

    QColor colore = id_colore_cursore.value(0).first.second;
    int pos = id_colore_cursore.value(0).second;


    fmt.setBackground(colore);

    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
    qDebug() << "testo from Start: " << cursor.selectedText();
    cursor.setCharFormat(fmt2);
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
     qDebug() << "testo to End: " << cursor.selectedText();
    cursor.setCharFormat(fmt2);
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
     qDebug() << "testo left: " << cursor.selectedText();
    cursor.setCharFormat(fmt);


    for(int i = 1; i < id_colore_cursore.size(); i++){
        QColor colore = id_colore_cursore.value(i).first.second;
        int pos = id_colore_cursore.value(i).second;

        fmt.setBackground(colore);

        cursor.setPosition(pos);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.setCharFormat(fmt2);
        cursor.setPosition(pos);
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
        cursor.setCharFormat(fmt);
    }

    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));

}




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

MainWindow::MainWindow(Socket *sock, FileHandler *fileHand,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    socket(sock),
    fHandler(fileHand),
    boldIsOn(false),
    underlinedIsOn(false),
    italicIsOn(false)
{
    ui->setupUi(this);
    setWindowTitle("Google Fake Docs");

    /*CONNECT per segnali uscenti, inoltrare le modifiche fatte*/
    connect( this, SIGNAL(myInsert(int, QChar, int, bool, bool, bool)),
              fHandler, SLOT(localInsert(int, QChar, int, bool, bool, bool)));
    connect( this, SIGNAL(myDelete(int)),
              fHandler, SLOT(localDelete(int)));
    connect( this, SIGNAL(sendNameFile(QString)),
              socket, SLOT(sendCheckFileName(QString)));
    connect( this, SIGNAL(newFile(QString)),
             socket, SLOT(sendNewFile(QString)));

    /*CONNECT per segnali entranti, applicare sulla GUI le modifiche che arrivano sul socket*/
    connect( socket, SIGNAL(readyInsert(QJsonArray, QChar, int, int, int)),
              fHandler,  SLOT(remoteInsert(QJsonArray, QChar, int, int, int)));
    connect( socket, SIGNAL(readyDelete(QString)),
              fHandler, SLOT(remoteDelete(QString)));
    connect( socket, SIGNAL(readyFile()),  this, SLOT(fileIsHere()));
    connect( fHandler, SIGNAL(readyRemoteInsert(QChar, int)),
             this, SLOT(changeViewAfterInsert(QChar, int)));
    connect( fHandler, SIGNAL(readyRemoteDelete(int)),
             this, SLOT(changeViewAfterDelete(int)));

    /*CONNECT per lo stile dei caratteri*/
    connect( this, SIGNAL(newStyle(QString, bool, int, int)),
              fHandler, SLOT(changeStyle(QString, bool, int, int)));
}

MainWindow::~MainWindow()
{
    delete ui;
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

void MainWindow::on_textEdit_cursorPositionChanged()
{
    QTextCursor cursor(ui->textEdit->textCursor());

    /*Se il testo selezionato ha stile misto, i bottoni accendono lo stile*/
    if(cursor.hasSelection()==true){
        int i=0;
        bool mixBold = false;
        bool mixUnderlined = false;
        bool mixItalic = false;
        auto vettore = this->fHandler->getVectorFile();
        this->boldIsOn = vettore.at(cursor.selectionStart())->getBoldBool();
        this->underlinedIsOn = vettore.at(cursor.selectionStart())->getUnderlinedBool();
        this->italicIsOn = vettore.at(cursor.selectionStart())->getItalicBool();

        /*Studio i caratteri*/
        for(i=cursor.selectionStart()+1; i<=cursor.selectionEnd()-1; i++){
            if(mixBold==false && (this->boldIsOn != vettore.at(i)->getBoldBool())) mixBold=true;
            if(mixUnderlined==false && (this->underlinedIsOn != vettore.at(i)->getUnderlinedBool())) mixUnderlined=true;
            if(mixItalic==false && (this->italicIsOn != vettore.at(i)->getItalicBool())) mixItalic=true;
        }

        /*Imposto lo stile*/
        if(mixBold == true) this->boldIsOn=false;
        if(mixUnderlined == true) this->underlinedIsOn=false;
        if(mixItalic == true) this->italicIsOn=false;
    }

    /*Catturiamo lo stile del carattere precendete*/
    else if((this->fHandler->getVectorFile().size() == ui->textEdit->toPlainText().size()) && cursor.position()>0){
        auto vettore = this->fHandler->getVectorFile();

        this->boldIsOn = vettore.at(cursor.position()-1)->getBoldBool();
        this->underlinedIsOn = vettore.at(cursor.position()-1)->getUnderlinedBool();
        this->italicIsOn = vettore.at(cursor.position()-1)->getItalicBool();

        /*Cambiando il grassetto*/
        if(this->boldIsOn == true){
            ui->textEdit->setFontWeight(75);
        }
        else {
            ui->textEdit->setFontWeight(50);
        }

        /*Cambiando il sottolineato*/
        if(this->underlinedIsOn==true)
            ui->textEdit->setFontUnderline(true);
        else
            ui->textEdit->setFontUnderline(false);

        /*Cambiando il corsivo*/
        if(this->italicIsOn == true)
            ui->textEdit->setFontItalic(true);
        else
            ui->textEdit->setFontItalic(false);

        qDebug() << "Catturato stile BUI: " << this->boldIsOn << this->underlinedIsOn << this->italicIsOn;
    }

    qDebug() << "LETTERE CONTENUTE: ";
    QString frase = "";
    for(auto v : this->fHandler->getVectorFile()){
        frase += v->getValue();
    }
    qDebug() << frase << "--- FINE";
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
        qDebug() << "Attivo/disattivo il grassetto";
        if(this->boldIsOn==true){
            this->boldIsOn=false;
            qDebug() << "Disattivo il grassetto";
            ui->textEdit->setFontWeight(50);
        }
        else {
            this->boldIsOn=true;
            qDebug() << "Attivo disattivo il grassetto";
            ui->textEdit->setFontWeight(75);
        }
    }

    /*CASO2: Cambio il grassetto di una selezione*/
    //Conteggio inizia da 0, quindi num_tot = selectionEnd-selectionStart
    //Pensare come cursore non come lettere selezioante
    //Es. ciao0 ciao1 ciao2
    //Se seleziono ciao0
    //selectionStart = 0
    //selectionEnd = 5

    else{
        disconnect(this, SIGNAL(myInsert(int, QChar, int, bool, bool, bool)),
                  fHandler, SLOT(localInsert(int, QChar, int, bool, bool, bool)));
        disconnect(this, SIGNAL(myDelete(int)),
                  fHandler, SLOT(localDelete(int)));

        qDebug() << "Seleziono un testo per grassetto";

        /*Inviare una notifica che lo stile è cambiato da selectionStart a selectionEnd*/
        if(this->boldIsOn == false) {
            //Setto grasetto
            ui->textEdit->setFontWeight(75);
            this->boldIsOn = true;
            emit newStyle("BOLD", 1, cursor.selectionStart(), cursor.selectionEnd()-1);

            //Controllo se sono sull'ultima lettera
        }
        else {
            //Disattivo grasetto
            ui->textEdit->setFontWeight(50);
            this->boldIsOn = false;
            emit newStyle("BOLD", 0, cursor.selectionStart(), cursor.selectionEnd()-1);
        }

        connect( this, SIGNAL(myInsert(int, QChar, int, bool, bool, bool)),
                  fHandler, SLOT(localInsert(int, QChar, int, bool, bool, bool)));
        connect( this, SIGNAL(myDelete(int)),
                  fHandler, SLOT(localDelete(int)));

    }
}

void MainWindow::on_actionItalic_triggered()
{
    auto cursor = ui->textEdit->textCursor();
    qDebug() << "Selection start: " << cursor.selectionStart() << " end: " << cursor.selectionEnd();

    /*CASO1: Non sto selezionando niente, attivo/disattivo il corsivo*/
    if(cursor.selectionStart() - cursor.selectionEnd() == 0){
        qDebug() << "Attivo/disattivo il corsivo";
        if(this->italicIsOn==true){
            this->italicIsOn=false;
            qDebug() << "Disattivo il corsivo";
            ui->textEdit->setFontItalic(false);
        }
        else {
            this->italicIsOn=true;
            qDebug() << "Attivo disattivo il corsivo";
            ui->textEdit->setFontItalic(true);
        }
    }

    else{
        disconnect(this, SIGNAL(myInsert(int, QChar, int, bool, bool, bool)),
                  fHandler, SLOT(localInsert(int, QChar, int, bool, bool, bool)));
        disconnect(this, SIGNAL(myDelete(int)),
                  fHandler, SLOT(localDelete(int)));

        qDebug() << "Seleziono un testo per corsivo";

        /*Inviare una notifica che lo stile è cambiato da selectionStart a selectionEnd*/
        if(this->italicIsOn == false) {
            //Setto corsivo
            ui->textEdit->setFontItalic(true);
            this->italicIsOn = true;
            emit newStyle("ITALIC", 1, cursor.selectionStart(), cursor.selectionEnd()-1);
        }
        else {
            //Disattivo corsivo
            ui->textEdit->setFontItalic(false);
            this->italicIsOn = false;
            emit newStyle("ITALIC", 0, cursor.selectionStart(), cursor.selectionEnd()-1);
        }

        connect( this, SIGNAL(myInsert(int, QChar, int, bool, bool, bool)),
                  fHandler, SLOT(localInsert(int, QChar, int, bool, bool, bool)));
        connect( this, SIGNAL(myDelete(int)),
                  fHandler, SLOT(localDelete(int)));

    }

}

void MainWindow::on_actionUnderlined_triggered()
{
    auto cursor = ui->textEdit->textCursor();
    qDebug() << "Selection start: " << cursor.selectionStart() << " end: " << cursor.selectionEnd();

    /*CASO1: Non sto selezionando niente, attivo/disattivo il sottolineato*/
    if(cursor.selectionStart() - cursor.selectionEnd() == 0){
        qDebug() << "Attivo/disattivo il sottolineato";
        if(this->underlinedIsOn==true){
            this->underlinedIsOn=false;
            qDebug() << "Disattivo il sottolineato";
            ui->textEdit->setFontUnderline(false);
        }
        else {
            this->underlinedIsOn=true;
            qDebug() << "Attivo disattivo il sottolineato";
            ui->textEdit->setFontUnderline(true);
        }
    }

    else{
        disconnect(this, SIGNAL(myInsert(int, QChar, int, bool, bool, bool)),
                  fHandler, SLOT(localInsert(int, QChar, int, bool, bool, bool)));
        disconnect(this, SIGNAL(myDelete(int)),
                  fHandler, SLOT(localDelete(int)));

        qDebug() << "Seleziono un testo per sottolineato";

        /*Inviare una notifica che lo stile è cambiato da selectionStart a selectionEnd*/
        if(this->underlinedIsOn == false) {
            //Setto sottolineato
            ui->textEdit->setFontUnderline(true);
            this->underlinedIsOn = true;
            emit newStyle("UNDERLINE", 1, cursor.selectionStart(), cursor.selectionEnd()-1);
        }
        else {
            //Disattivo sottolineato
            ui->textEdit->setFontUnderline(false);
            this->underlinedIsOn = false;
            emit newStyle("UNDERLINE", 0, cursor.selectionStart(), cursor.selectionEnd()-1);
        }

        connect( this, SIGNAL(myInsert(int, QChar, int, bool, bool, bool)),
                  fHandler, SLOT(localInsert(int, QChar, int, bool, bool, bool)));
        connect( this, SIGNAL(myDelete(int)),
                  fHandler, SLOT(localDelete(int)));

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
        letterCounter++;
    //ui->statusBar->showMessage(c);
        emit myInsert(externalIndex, newLetterValue, socket->getClientID(), this->boldIsOn, this->italicIsOn, this->underlinedIsOn);
    }
    else{  /*Testo cambiato con DELETE */
        letterCounter--;
        emit myDelete(externalIndex+1);
    }
}



void MainWindow::on_lineEdit_editingFinished()
{
    /*Cambio il nome del documento, solo dopo l'OK*/
    //emit sendNameFile(ui->lineEdit->text());
}

void MainWindow::fileIsHere(){
    qDebug() << "FileIsHere";
    disconnect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));

    /*Aggiornare la GUI con il file appena arrivato*/
    QVector<Letter*> vectorFile = this->fHandler->getVectorFile();
    QString text = "";
    for(Letter *l : vectorFile){
        QChar c = l->getValue();
        letterCounter++;
        text.append(c);
    }

    ui->textEdit->setText(text);
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
}

void MainWindow::changeViewAfterInsert(QChar l, int pos)
{
    disconnect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));

//    QTextCursor cursor(ui->textEdit->textCursor());
//    cursor.setPosition(pos);
//    ui->textEdit->(l);
//    letterCounter++;

    QVector<Letter*> vectorFile = this->fHandler->getVectorFile();
    QString text = "";
    for(Letter *l : vectorFile){
        QChar c = l->getValue();
        letterCounter++;
        text.append(c);
    }

    ui->textEdit->setText(text);
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
}

void MainWindow::changeViewAfterDelete(int externalIndex)
{
    disconnect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));

    QVector<Letter*> vectorFile = this->fHandler->getVectorFile();
    QString text = "";
    for(Letter *l : vectorFile){
        QChar c = l->getValue();
        letterCounter++;
        text.append(c);
    }

    ui->textEdit->setText(text);
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
}


/*void MainWindow::setCursor(int pos, QString color)
{
    QString cursore = "|";
    QString colore = "<span style=\" font-size:8pt; font-weight:600; color:" + color + '"' + ";\" >";
    colore.append(cursore);
    colore.append("</span>");
    QTextCursor c;
    c.setPosition(pos);
    c.insertHtml(colore);
}*/

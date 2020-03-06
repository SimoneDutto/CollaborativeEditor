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

MainWindow::MainWindow(Socket *sock, FileHandler *fileHand,QWidget *parent, QString nome) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    socket(sock),
    fHandler(fileHand)
{
    ui->setupUi(this);
    QPalette pal = palette();

    // set black background
    pal.setColor(QPalette::Background, QColor(128,128,128));
    pal.setColor(QPalette::WindowText, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->show();

    // set picture
    QPixmap pix("path -- TO DO");
    ui->label_pic->setPixmap(pix);

    setWindowTitle(nome);
    //ui->lineEdit->setText(nome);

    /*CONNECT per segnali uscenti, inoltrare le modifiche fatte*/
    connect( this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat)),
              fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat)));
    connect( this, SIGNAL(myDelete(int)),
              fHandler, SLOT(localDelete(int)));
    connect( this, SIGNAL(sendNameFile(QString)),
              socket, SLOT(sendCheckFileName(QString)));
    connect( this, SIGNAL(newFile(QString)),
             socket, SLOT(sendNewFile(QString)));

    /*CONNECT per segnali entranti, applicare sulla GUI le modifiche che arrivano sul socket*/

    connect( socket, SIGNAL(readyInsert(QJsonArray, QChar, int, int, int, QTextCharFormat)),
              fHandler,  SLOT(remoteInsert(QJsonArray, QChar, int, int, int, QTextCharFormat)));
    connect( socket, SIGNAL(readyDelete(QString)),
              fHandler, SLOT(remoteDelete(QString)));
    connect( socket, SIGNAL(readyFile()),  this, SLOT(fileIsHere()));
    connect( fHandler, SIGNAL(readyRemoteInsert(QChar, int, QTextCharFormat)),
             this, SLOT(changeViewAfterInsert(QChar, int, QTextCharFormat)));
    connect( fHandler, SIGNAL(readyRemoteDelete(int)),
             this, SLOT(changeViewAfterDelete(int)));
    connect( fHandler, SIGNAL(readyRemoteStyleChange(QString, QString)),
             this, SLOT(changeViewAfterStyle(QString, QString)));
    connect( socket, SIGNAL(readyStyleChange(QString, QString, QString)),
             fHandler, SLOT(remoteStyleChange(QString, QString, QString)));

    /*CONNECT per lo stile dei caratteri*/
    connect( this, SIGNAL(styleChange(QMap<QString, QTextCharFormat>)),
              fHandler, SLOT(localStyleChange(QMap<QString, QTextCharFormat>)) );
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
        disconnect(this, SIGNAL(myDelete(int)),
                  fHandler, SLOT(localDelete(int)));

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

        //QString startID = vettore.at(start)->getLetterID();
        //QString finalID = vettore.at(end)->getLetterID();

        /* Se non dovesse servire la mappa basta prendere i due id iniziali e finali
           QString idIniziale =  vettore.at(cursor.selectionStart())
           QString idFinale =  vettore.at(cursor.selectionEnd()-1)
           e poi inviarli con la emit() */

        for(i=start; i<=end; i++){
            cursor.setPosition(i+1);
            auto letterFormat = cursor.charFormat();
            qDebug() << letterFormat.fontWeight() << "---" << letterFormat.fontUnderline() << "---" << letterFormat.fontItalic();
            //vettore.at(i)->setFormat(letterFormat);
            formatCharMap.insert(vettore.at(i)->getLetterID(), letterFormat);
        }

        emit styleChange(formatCharMap);

        connect( this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat)));
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
        if(ui->textEdit->fontItalic()!=true)
            ui->textEdit->setFontItalic(true);
        else
            ui->textEdit->setFontItalic(false);
    }

    /*CASO2: Cambio il corsivo di una selezione*/
    else{
        disconnect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat)));
        disconnect(this, SIGNAL(myDelete(int)),
                  fHandler, SLOT(localDelete(int)));

        qDebug() << "Seleziono un testo per corsivo";

        if(ui->textEdit->fontItalic()!=true)
            ui->textEdit->setFontItalic(true);
        else
            ui->textEdit->setFontItalic(false);

        /* Aggiorno il modello */
        QMap<QString, QTextCharFormat> formatCharMap;
        auto vettore = this->fHandler->getVectorFile();
        int i=0;

        int start = cursor.selectionStart();
        int end = cursor.selectionEnd()-1;

        for(i=start; i<=end; i++){
            cursor.setPosition(i+1);
            auto letterFormat = cursor.charFormat();
            qDebug() << letterFormat.fontWeight() << "---" << letterFormat.fontUnderline() << "---" << letterFormat.fontItalic();
            //vettore.at(i)->setFormat(letterFormat);
            formatCharMap.insert(vettore.at(i)->getLetterID(), letterFormat);
        }

        emit styleChange(formatCharMap);

        connect( this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat)));
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
        if(ui->textEdit->fontUnderline()!=true)
            ui->textEdit->setFontUnderline(true);
        else
            ui->textEdit->setFontUnderline(false);
    }

    /*CASO2: Cambio il sottolineato di una selezione*/
    else{
        disconnect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat)));
        disconnect(this, SIGNAL(myDelete(int)),
                  fHandler, SLOT(localDelete(int)));

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

        for(i=start; i<=end; i++){
            /* Se testo selezionato misto, allora settare, altrimenti se tutto settato, togliere */
            cursor.setPosition(i+1);
            auto letterFormat = cursor.charFormat();
            qDebug() << letterFormat.fontWeight() << "---" << letterFormat.fontUnderline() << "---" << letterFormat.fontItalic();
            //vettore.at(i)->setFormat(letterFormat);
            formatCharMap.insert(vettore.at(i)->getLetterID(), letterFormat);
        }

        emit styleChange(formatCharMap);

        connect( this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat)));
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
        emit myInsert(externalIndex, newLetterValue, socket->getClientID(), cursor.charFormat());
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
    QTextCursor cursor(ui->textEdit->textCursor());
    cursor.setPosition(0);
    auto vettore = this->fHandler->getVectorFile();

    for(auto lettera : vettore){
        cursor.insertText(lettera->getValue(), lettera->getFormat());
    }

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

//TODO: inserire gestione bottoni
void MainWindow::on_textEdit_cursorPositionChanged() {

    /*Questa funzione gestirà la vista dei bottoni dello stile, ovvero se si vedrenno accessi o spenti. */

    QTextCursor cursor(ui->textEdit->textCursor());

    /*Se il testo selezionato ha stile misto, i bottoni accendono lo stile*/
    if(cursor.hasSelection()==true){
        if(ui->textEdit->fontWeight()!=75){
            //BoldButton OFF
        }
        else {}//BoldButton ON

        if(ui->textEdit->fontItalic()!=true){
            //ItalicButton OFF
        }
        else {}//ItalicButton ON

        if(ui->textEdit->fontUnderline()!=true){
            //UnderlineButton OFF
        }
        else {}//UnderlineButton ON

    }


    /*Catturiamo lo stile del carattere precendete per settare i bottoni ON/OFF*/
    else {
        auto format = cursor.charFormat();

        if(format.fontWeight()==55){
            //BoldButton OFF
        }
        else {} //BoldButton ON

        if(format.fontItalic()==false){
            //ItalicButton OFF
        }
        else {} //ItalicButton ON

        if(format.fontUnderline()==false){
            //UnderlineButton OFF
        }
        else {} //UnderlineButton ON
    }
}

void MainWindow::on_actionLog_Out_triggered()
{
    /*qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());*/
}

void MainWindow::on_actionEdit_Profile_triggered()
{
    account = new Account(this->socket, this);
    hide();
    account->show();

}

void MainWindow::on_actionGet_URI_triggered()
{
    // ricavare URI da passare al costruttore
    uri = new Uri(socket,this,"QUI USCIRA' L'URI");
    uri->show();
}

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

MainWindow::MainWindow(Socket *sock, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    socket(sock),
    fHandler(sock->getFHandler())
{
    ui->setupUi(this);
    setWindowTitle("Notepad dei Povery");

    /*
void Socket::updateLocalInsert(int externalIndex, QChar newLetterValue){
    this->fileh->localInsert(externalIndex, newLetterValue, this->clientID);
}
*/

    /*CONNECT per segnali uscenti, inoltrare le modifiche fatte*/
    connect( this, SIGNAL(myInsert(int externalIndex, QChar newLetterValue, int clientID)),
             fHandler, SLOT(localInsert(int externalIndex, QChar newLetterValue, int clientID)));
    connect( this, SIGNAL(myDelete(int externalIndex)),
             fHandler, SLOT(localDelete(int externalIndex)));
    connect( this, SIGNAL(sendNameFile(QString fileNameTmp);),
             socket, SLOT(sendCheckFileName(QString fileNameTmp)));
    connect( this, SIGNAL(newFile();),
             socket, SLOT(sendNewFile()));


    /*CONNECT per segnali entranti, applicare sulla GUI le modifiche che arrivano sul socket*/
    connect( socket, SIGNAL(readyInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter)),
             fHandler,  SLOT(remoteInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter)));
    connect( socket, SIGNAL(readyDelete(QString deletedLetterID)),
             fHandler, SLOT(remoteDelete(QString deletedLetterID)));
    connect( socket, SIGNAL(readyFile()),  SLOT(fileIsHere()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionNew_triggered()
{
    ui->textEdit->setText("");
    ui->lineEdit->setText("Nuovo Documento");
    emit newFile();
}

void MainWindow::on_actionOpen_triggered()
{
    //QString file_name = QFileDialog::getOpenFileName(this,"Open the file");
    dialog = new Dialog(this->socket, this->fHandler, this);
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
    if(ui->textEdit->fontWeight()!=75)
        ui->textEdit->setFontWeight(75);
    else
        ui->textEdit->setFontWeight(50);
}

void MainWindow::on_actionItalic_triggered()
{
    if(ui->textEdit->fontItalic())
        ui->textEdit->setFontItalic(false);
    else
        ui->textEdit->setFontItalic(true);
}

void MainWindow::on_actionUnderlined_triggered()
{
    if(ui->textEdit->fontUnderline())
        ui->textEdit->setFontUnderline(false);
    else
        ui->textEdit->setFontUnderline(true);
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
    /*Testo cambiato con INSERT o DELETE?*/

    QTextCursor cursor(ui->textEdit->textCursor());
    int externalIndex = cursor.position();
    //ui->statusBar->showMessage(QString::number(pos));
    cursor.select(QTextCursor::LineUnderCursor);
    QChar newLetterValue = cursor.selectedText().right(1).at(0);
    //ui->statusBar->showMessage(c);

    emit myInsert(externalIndex, newLetterValue, socket->getClientID());
    emit myDelete(externalIndex);
}


void MainWindow::on_lineEdit_editingFinished()
{
    /*Cambio il nome del documento, solo dopo l'OK*/
    emit sendNameFile(ui->lineEdit->text());
}

void MainWindow::fileIsHere(){
    /*Aggiornare la GUI con il file appena arrivato*/
    QVector<Letter> vectorFile = this->fHandler->getVectorFile();

}

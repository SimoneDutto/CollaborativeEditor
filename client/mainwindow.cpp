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
#include <QComboBox>
#include <QFontComboBox>
#include <algorithm>
#include <QFontDatabase>
#include <QTextBlock>
#include <QTextDocumentFragment>
#include <QThread>


MainWindow::MainWindow(Socket *sock, QWidget *parent, QString nome) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    socket(sock)
{
    socket->createFileHandler();
    fHandler = socket->getFHandler();
    ui->setupUi(this);
    // set black background
    pal.setColor(QPalette::Background, QColor(58,58,60));
    pal.setColor(QPalette::WindowText, Qt::white);
    pal.setColor(QPalette::ButtonText, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->show();

    QPalette p = ui->textEdit->palette(); // define pallete for textEdit..
    p.setColor(QPalette::Base, Qt::white); // set color "Red" for textedit base
    p.setColor(QPalette::Text, Qt::black); // set text color which is selected from color pallete
    ui->textEdit->setPalette(p);
    ui->textEdit->setStyleSheet("QTextEdit { padding:20}");
    ui->textEdit->setAcceptRichText(true);
    // set picture

    QIcon *discard_icon= new QIcon(":/rec/icone/icons8-punta-della-matita-96.png");
    ui->discardImage->setIcon(*discard_icon);
    ui->discardImage->setIconSize(QSize(18, 18));

    QString styleSheet = "QPushButton {background-color: white; border-style: solid; border-width: 1px; border-radius: 15px; border-color: rgb(0, 0, 0);} QPushButton:hover {background-color: rgb(233, 233, 233)} QPushButton:pressed {background-color: rgb(181, 181, 181)}";
    ui->discardImage->setStyleSheet(styleSheet);

    setWindowTitle(nome);
    ui->label_2->setStyleSheet("background-color:lightgray; color:black");
    ui->username->setStyleSheet("color:white");
    ui->label_2->setTextInteractionFlags(Qt::TextSelectableByMouse);

    auto availableSize = qApp->desktop();
       int width = availableSize->width();
       int height = availableSize->height();
       width *= 0.7; // 80% of the screen size
       height *= 0.7; // 80% of the screen size
       QSize newSize( width, height );

       setGeometry(
           QStyle::alignedRect(
               Qt::LeftToRight,
               Qt::AlignCenter,
               newSize,
               qApp->desktop()->rect()
           )
       );

    /* Personalizzo e aggiungo le label degli utenti connessi */
    styleSheet = QString("QGroupBox {border: 0px;}");
    ui->groupBox->setStyleSheet(styleSheet);

    ui->user1->hide();
    ui->user2->hide();
    ui->user3->hide();
    ui->counter->hide();
    styleSheet = "QPushButton {color: black; background-color: white; border-style: solid; border-width: 2px; border-radius: 15px; border-color: black;} QPushButton:hover {background-color: rgb(233, 233, 233)} QPushButton:pressed {background-color: rgb(181, 181, 181)}";
    ui->counter->setStyleSheet(styleSheet);

    /* Etichette degli utenti online */
    ui->l_user1->hide();
    ui->l_user1->setStyleSheet("color:white");
    ui->l_user2->hide();
    ui->l_user2->setStyleSheet("color:white");
    ui->l_user3->hide();
    ui->l_user3->setStyleSheet("color:white");

    /* Aggiungo nome e icona dell'utente */

    QString username = socket->getClientUsername();
    ui->username->setText(username);

    styleSheet = "QLabel { background-color: rgb(255, 252, 247); color: black; border-style: solid; border-width: 2px; border-radius: 6px; border-color: orange; font: ; }";
    ui->myicon->setStyleSheet(styleSheet);
    QFont font("Arial", 30);
    ui->myicon->setFont(font);

    QString imageName = QString::number(socket->getClientID())+".png";
    QPixmap userPixmap = QPixmap(imageName);

    if(userPixmap != QPixmap()){
        QPixmap scaled = userPixmap.scaled(ui->myicon->width(), ui->myicon->height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        ui->myicon->setPixmap(scaled);
    }

    else {
        ui->myicon->setText(username.at(0).toUpper());
    }

    /* Aggiungo ComboBox e SizeBox */
    QAction *rightAll = ui->mainToolBar->actions().at(7);

    QComboBox* sizeComboBox = new QComboBox;
    QStringList* numList = new QStringList;

    for(int i=1; i<101; i++){
        numList->append(QString::number(i));
    }
    sizeComboBox->addItems(*numList);
    sizeComboBox->setStyleSheet("combobox-popup: 0;");
    sizeComboBox->setCurrentIndex(12);


    QFontComboBox* fontComboBox = new QFontComboBox;
    QFont init_font;
    init_font.setFamily("Arial");
    fontComboBox->setCurrentFont(init_font);

    ui->mainToolBar->insertWidget(rightAll, fontComboBox);
    ui->mainToolBar->insertWidget(rightAll, sizeComboBox);

    QFont currFont = ui->textEdit->currentFont();
    currFont.setFamily("Arial");
    currFont.setPointSize(13);
    ui->textEdit->setFont(currFont);


    /* CONNECT per collegare Font e Size */
    connect( fontComboBox, SIGNAL(currentFontChanged(QFont)),
             this, SLOT(currentFontChanged(QFont)));
    connect( this, SIGNAL(setCurrFont(QFont)),
             fontComboBox, SLOT(setCurrentFont(QFont)));
    connect( sizeComboBox, SIGNAL(currentIndexChanged(int)),
             this, SLOT(fontSizeChanged(int)));
    connect( this, SIGNAL(setCurrFontSize(int)),
             sizeComboBox, SLOT(setCurrentIndex(int)));

    /* CONNECT per segnali uscenti, inoltrare le modifiche fatte */
    connect( this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
              fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
    connect( this, SIGNAL(myDelete(int,int)),
              fHandler, SLOT(localDelete(int,int)));
    connect( this, SIGNAL(sendNameFile(QString)),
              socket, SLOT(sendCheckFileName(QString)));
    connect( this, SIGNAL(newFile(QString)),
             socket, SLOT(sendNewFile(QString)));
    connect( this, SIGNAL(sendHist()), socket, SLOT(sendHistory()));

    /* CONNECT per segnali entranti, applicare sulla GUI le modifiche che arrivano sul socket */

    connect( socket, SIGNAL(readyInsert(QJsonArray, QChar, int, int, int, QTextCharFormat, Qt::AlignmentFlag, bool, QString, QJsonArray)),
              fHandler,  SLOT(remoteInsert(QJsonArray, QChar, int, int, int, QTextCharFormat, Qt::AlignmentFlag, bool, QString, QJsonArray)));
    connect( socket, SIGNAL(readyDelete(QString)),
              fHandler, SLOT(remoteDelete(QString)));
    connect( socket, SIGNAL(readyFile(QMap<int,int>,QMap<int,QColor>)),  this, SLOT(fileIsHere(QMap<int,int>,QMap<int,QColor>)));
    connect( fHandler, SIGNAL(readyRemoteInsert(QChar, int, QTextCharFormat, Qt::AlignmentFlag, QString)),
             this, SLOT(changeViewAfterInsert(QChar, int, QTextCharFormat, Qt::AlignmentFlag, QString)));
    connect( fHandler, SIGNAL(readyRemoteDelete(int)),
             this, SLOT(changeViewAfterDelete(int)));
    connect( socket, SIGNAL(UserConnect(QString, QColor)),
             this, SLOT(addUserConnection(QString, QColor)));
    connect( socket, SIGNAL(UserDisconnect(QString,int)),
             this, SLOT(removeUserDisconnect(QString,int)));
    connect( socket, SIGNAL(writeURI(QString)),
             this, SLOT(on_write_uri(QString)));
    connect( socket, SIGNAL(HistorySuccess(QMap<int, QString>)),
             this, SLOT(uploadHistory(QMap<int, QString>)));
    connect( ui->textEdit, SIGNAL(pastedText(QString, QString)),
                 this, SLOT(insertPastedText(QString, QString)));

    /* CONNECT per lo stile dei caratteri */
    connect( this, SIGNAL(styleChange(QMap<QString, QTextCharFormat>)),
              fHandler, SLOT(localStyleChange(QMap<QString, QTextCharFormat>)) );
    connect( fHandler, SIGNAL(readyRemoteStyleChange(QString, QString)),
             this, SLOT(changeViewAfterStyle(QString, QString)));
    connect( socket, SIGNAL(readyStyleChange(QString, QString, QString, QString)),
             fHandler, SLOT(remoteStyleChange(QString, QString, QString, QString)));

    connect( this, SIGNAL(sendAlignment(Qt::AlignmentFlag,int,QString,QString)),
             fHandler, SLOT(localAlignChange(Qt::AlignmentFlag,int,QString,QString)));
    connect( socket, SIGNAL(readyAlignChange(Qt::AlignmentFlag,int,QString,QString)),
             fHandler, SLOT(remoteAlignChange(Qt::AlignmentFlag,int,QString,QString)));
    connect( fHandler, SIGNAL(readyRemoteAlignChange(Qt::AlignmentFlag,int)),
             this, SLOT(changeAlignment(Qt::AlignmentFlag,int)));

    connect( this, SIGNAL(sendColorChange(QString,QString,QString)),
             socket, SLOT(sendColor(QString,QString,QString)));
    connect( socket, SIGNAL(colorChange(QString,QString,QColor)),
             fHandler, SLOT(remoteColorChange(QString,QString,QColor)));
    connect( fHandler, SIGNAL(readyRemoteColorChange(int,int,QColor)),
             this, SLOT(changeViewAfterColor(int,int,QColor)));

    /* CONNECT per cursore */
    connect( socket, SIGNAL(userCursor(QPair<int,int>,QColor,QString)),
             this, SLOT(on_cursor_triggered(QPair<int,int>,QColor, QString)));
    connect( this, SIGNAL(sendCursorChange(int)),
             fHandler, SLOT(localCursorChange(int)));
    connect( this, SIGNAL(sendCursorSelection(int,int)),
             socket, SLOT(sendCursorSelectionToServer(int,int)));
    //connect( socket, SIGNAL(cursorSelection(int,int,QColor)),
             //this, SLOT(changeViewAfterSelection(int,int,QColor)));

    /* CONNECT per collegare le ClickableLabel */
    connect( ui->user1, SIGNAL(clicked()),
             this, SLOT(on_counter_clicked()));
    connect( ui->user2, SIGNAL(clicked()),
             this, SLOT(on_counter_clicked()));
    connect( ui->user3, SIGNAL(clicked()),
             this, SLOT(on_counter_clicked()));
    connect( ui->discardImage, SIGNAL(clicked()),
             this, SLOT(on_actionEdit_Profile_triggered()));
    connect( this, SIGNAL(openThisFile(QString)),
             this->socket, SLOT(sendOpenFile(QString)));
    connect(socket, SIGNAL(fileCreated(QString)), this, SLOT(changeTitle(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete fHandler;
}

void MainWindow::changeTitle(QString name){
    setWindowTitle(name);
}

bool MainWindow::sorting(QPair<QPair<int,QColor>,int> &e1, QPair<QPair<int,QColor>,int> &e2){
if(e1.second < e2.second)
return true;
else return false;
}

void MainWindow::on_actionNew_triggered()
{
    form = new Form(this->socket, this);
    form->show();
    //ui->lineEdit->setText(filename);
}

void MainWindow::on_actionOpen_triggered()
{
    dialog = new Dialog(this->socket, this);
    dialog->show();
}


void MainWindow::on_actionAbout_us_triggered()
{
    QString about_text;
      about_text  = "Authors: Debora Caldarola, Simone Dutto, Isabella Romita, Vito Tassielli\n";
      about_text += "Date: 11/09/2019\n";
      about_text += "(C) C++ollaborative Editor  (R)\n";

      QMessageBox::about(this,"About C++ollaborative Editor",about_text);
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

    /* CASO1: Non sto selezionando niente, attivo/disattivo il grassetto */
    if(cursor.selectionStart() - cursor.selectionEnd() == 0){
        if(ui->textEdit->fontWeight()!=75)
            ui->textEdit->setFontWeight(75);
        else
            ui->textEdit->setFontWeight(50);
    }

    /* CASO2: Cambio il grassetto di una selezione */
    else{
        disconnect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
        disconnect(this, SIGNAL(myDelete(int,int)),
                  fHandler, SLOT(localDelete(int,int)));

        qDebug() << "Seleziono un testo per grassetto";

        /* Aggiorno il modello */
        QMap<QString, QTextCharFormat> formatCharMap;
        auto vettore = this->fHandler->getVectorFile();
        int i=0;

        int start = cursor.selectionStart();
        int end = cursor.selectionEnd()-1;

        if(isSelectionBold(start, end+1))
            ui->textEdit->setFontWeight(50);
        else
            ui->textEdit->setFontWeight(75);

        QString startID = vettore.at(start)->getLetterID();
        QString lastID = vettore.at(end)->getLetterID();

        for(i=start; i<=end; i++){
            cursor.setPosition(i+1);
            auto letterFormat = cursor.charFormat();
            qDebug() << letterFormat.font() << letterFormat.fontWeight() << "---" << letterFormat.fontUnderline() << "---" << letterFormat.fontItalic();
            qDebug() << "LetterID = " << vettore.at(i)->getLetterID();
            formatCharMap.insert(vettore.at(i)->getLetterID(), letterFormat);
        }

        emit styleChange(formatCharMap);

        connect( this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
        connect( this, SIGNAL(myDelete(int,int)),
                  fHandler, SLOT(localDelete(int,int)));
    }
}

bool MainWindow::isSelectionBold(int start, int end) {
    QVector<Letter*> text = this->fHandler->getVectorFile();
    for(int i=start; i<end; i++) {
        if(text[i]->getFormat().fontWeight()==50)
            return false;
    }
    return true;
}

bool MainWindow::isSelectionItalic(int start, int end) {
    QVector<Letter*> text = this->fHandler->getVectorFile();
    for(int i=start; i<end; i++) {
        if(!text[i]->getFormat().fontItalic())
            return false;
    }
    return true;
}

bool MainWindow::isSelectionUnderlined(int start, int end) {
    QVector<Letter*> text = this->fHandler->getVectorFile();
    for(int i=start; i<end; i++) {
        if(!text[i]->getFormat().fontUnderline())
            return false;
    }
    return true;
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
        disconnect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
        disconnect(this, SIGNAL(myDelete(int,int)),
                  fHandler, SLOT(localDelete(int,int)));

        qDebug() << "Seleziono un testo per corsivo";
        qDebug() << ui->textEdit->fontItalic();

        /* Aggiorno il modello */
        QMap<QString, QTextCharFormat> formatCharMap;
        auto vettore = this->fHandler->getVectorFile();
        int i=0;

        int start = cursor.selectionStart();
        int end = cursor.selectionEnd()-1;

        if(!isSelectionItalic(start, end+1))
            ui->textEdit->setFontItalic(true);
        else
            ui->textEdit->setFontItalic(false);

        QString startID = vettore.at(start)->getLetterID();
        QString lastID = vettore.at(end)->getLetterID();

        for(i=start; i<=end; i++){
            cursor.setPosition(i+1);
            auto letterFormat = cursor.charFormat();
            qDebug() << letterFormat.fontWeight() << "---" << letterFormat.fontUnderline() << "---" << letterFormat.fontItalic();
            qDebug() << "LetterID = " << vettore.at(i)->getLetterID();

            formatCharMap.insert(vettore.at(i)->getLetterID(), letterFormat);
        }

        emit styleChange(formatCharMap);

        connect( this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
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
        disconnect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
        disconnect(this, SIGNAL(myDelete(int,int)),
                  fHandler, SLOT(localDelete(int,int)));

        qDebug() << "Seleziono un testo per sottolineato";

        /* Aggiorno il modello */
        QMap<QString, QTextCharFormat> formatCharMap;
        auto vettore = this->fHandler->getVectorFile();
        int i=0;

        int start = cursor.selectionStart();
        int end = cursor.selectionEnd()-1;

        if(!isSelectionUnderlined(start, end+1))
            ui->textEdit->setFontUnderline(true);
        else
            ui->textEdit->setFontUnderline(false);

        QString startID = vettore.at(start)->getLetterID();
        QString lastID = vettore.at(end)->getLetterID();

        for(i=start; i<=end; i++){
            /* Se testo selezionato misto, allora settare, altrimenti se tutto settato, togliere */
            cursor.setPosition(i+1);
            auto letterFormat = cursor.charFormat();
            qDebug() << letterFormat.fontWeight() << "---" << letterFormat.fontUnderline() << "---" << letterFormat.fontItalic();
            formatCharMap.insert(vettore.at(i)->getLetterID(), letterFormat);
        }

        emit styleChange(formatCharMap);

        connect( this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
        connect( this, SIGNAL(myDelete(int,int)),
                  fHandler, SLOT(localDelete(int,int)));
    }
}

void MainWindow::on_actionFont_triggered()
{
    bool ok;

    QFont font = QFontDialog::getFont(&ok, this);
    if(!ok)
        return;
    ui->textEdit->setFont(font);
    qDebug() << "Font: " << font.toString();
}

void MainWindow::on_actionColor_triggered()
{
    auto cursor = ui->textEdit->textCursor();
    qDebug() << "Selection start: " << cursor.selectionStart() << " end: " << cursor.selectionEnd();

    disconnect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
              fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
    disconnect(this, SIGNAL(myDelete(int,int)),
              fHandler, SLOT(localDelete(int,int)));

    QColor color = QColorDialog::getColor(Qt::white,this,"Choose a color");
    if(color.isValid())
        ui->textEdit->setTextColor(color);

    /* Aggiorno il modello */
    auto vettore = this->fHandler->getVectorFile();
    int i=0;

    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();

    if(start != end) {
        end--;
        QString startID, lastID;

        lastID = vettore.at(end)->getLetterID();

        if(vettore.size() > start) {
            startID = vettore.at(start)->getLetterID();
            if(start == end+1)
                lastID = startID;
        } else startID = lastID;

        for(i=start; i<=end; i++){
            cursor.setPosition(i+1);
            vettore.at(i)->setColor(color);
        }

        emit sendColorChange(startID, lastID, color.name());
    }

    connect( this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
              fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
    connect( this, SIGNAL(myDelete(int,int)),
              fHandler, SLOT(localDelete(int,int)));
}


void MainWindow::on_actionBackground_Color_triggered()
{
    auto cursor = ui->textEdit->textCursor();
    qDebug() << "Selection start: " << cursor.selectionStart() << " end: " << cursor.selectionEnd();

    disconnect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
              fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
    disconnect(this, SIGNAL(myDelete(int,int)),
              fHandler, SLOT(localDelete(int,int)));

    QColor color = QColorDialog::getColor(Qt::white,this,"Choose a color");
    if(color.isValid())
        ui->textEdit->setTextBackgroundColor(color);

    /* Aggiorno il modello */
    auto vettore = this->fHandler->getVectorFile();
    int i=0;

    int start = cursor.selectionStart();
    int end = cursor.selectionEnd()-1;

    QString startID = vettore.at(start)->getLetterID();
    QString lastID = vettore.at(end)->getLetterID();

    for(i=start; i<=end; i++){
        cursor.setPosition(i+1);
        vettore.at(i)->setBack(color);
    }

    connect( this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
              fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
    connect( this, SIGNAL(myDelete(int,int)),
              fHandler, SLOT(localDelete(int,int)));
}


void MainWindow::on_textEdit_textChanged()
{
    QTextCursor cursor(ui->textEdit->textCursor());
    int externalIndex = cursor.position();
    int numberOfLetters = ui->textEdit->toPlainText().size();
    qDebug() << "here" << numberOfLetters << letterCounter;

    if(numberOfLetters > letterCounter) {   // Compare actual number of letters in editor to the previous situation
        /* Testo cambiato con INSERT */
        qDebug() << "!!!!!!!!!!!!!!!!!!!!!insert";
        //qDebug() << "N" << receivers(SIGNAL(myInsert(int,QChar,int,QTextCharFormat,Qt::AlignmentFlag)));
        if (receivers(SIGNAL(myInsert(int,QChar,int,QTextCharFormat,Qt::AlignmentFlag))) > 0) {
            QChar newLetterValue = ui->textEdit->toPlainText().at(externalIndex-1);
            letterCounter++;
            emit myInsert(externalIndex, newLetterValue, socket->getClientID(), cursor.charFormat(), this->getFlag(ui->textEdit->alignment()));

            emit sendCursorChange(externalIndex);
            disconnect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
            QTextCharFormat fmt2;
            fmt2.setBackground(Qt::white);

            cursor.setPosition(externalIndex);
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
            //qDebug() << "testo from Start: " << cursor.selectedText();
            cursor.mergeCharFormat(fmt2);
            connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));

        }
    }
    else if (numberOfLetters <= letterCounter){
        /* Testo cambiato con DELETE */
        // lettere consecutive => basta trovare la differenza delle dimensioni
        int deletedLetters = letterCounter - numberOfLetters;
        if (receivers(SIGNAL(myDelete(int,int))) > 0) {
            qDebug() << "!!!!!!!!!!!!!!!!!!!!!delete";
            letterCounter -= deletedLetters;
            emit myDelete(externalIndex+1, externalIndex+deletedLetters);
        }
        if(ui->textEdit->toPlainText().size()==0){
            ui->textEdit->setCurrentCharFormat(this->firstLetter);

            Qt::Alignment currAlign = ui->textEdit->alignment();

            if(currAlign.testFlag(Qt::AlignLeft)){
                ui->actionAlign_to_Left->setChecked(true);
                ui->actionAlign_to_Right->setChecked(false);
                ui->actionAlign_to_Center->setChecked(false);
                ui->actionAlign_to_Justify->setChecked(false);
            }

            else if(currAlign.testFlag(Qt::AlignRight)){
                ui->actionAlign_to_Left->setChecked(false);
                ui->actionAlign_to_Right->setChecked(true);
                ui->actionAlign_to_Center->setChecked(false);
                ui->actionAlign_to_Justify->setChecked(false);
            }

            else if(currAlign.testFlag(Qt::AlignCenter)){
                ui->actionAlign_to_Left->setChecked(false);
                ui->actionAlign_to_Right->setChecked(false);
                ui->actionAlign_to_Center->setChecked(true);
                ui->actionAlign_to_Justify->setChecked(false);
            }

            else if(currAlign.testFlag(Qt::AlignJustify)){
                ui->actionAlign_to_Left->setChecked(false);
                ui->actionAlign_to_Right->setChecked(false);
                ui->actionAlign_to_Center->setChecked(false);
                ui->actionAlign_to_Justify->setChecked(true);
            }

            emit setCurrFont(this->firstLetter.font());
            emit setCurrFontSize(this->firstLetter.font().pointSize()-1);
        }
    }
}

Qt::AlignmentFlag MainWindow::getFlag(Qt::Alignment a) {
    if(a.testFlag(Qt::AlignLeft))
        return Qt::AlignLeft;
    else if(a.testFlag(Qt::AlignRight))
        return Qt::AlignRight;
    else if(a.testFlag(Qt::AlignCenter) || a.testFlag(Qt::AlignHCenter))
        return Qt::AlignCenter;
    else return Qt::AlignJustify;
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
    this->fHandler = this->socket->getFHandler();
    auto vettore = this->fHandler->getVectorFile();

    Qt::AlignmentFlag alignment;
    bool first = true;

    for(auto lettera : vettore){
        cursor.insertText(lettera->getValue(), lettera->getFormat());
        alignment = lettera->getAlignment();
        QTextBlockFormat blockFormat = cursor.blockFormat();
        blockFormat.setAlignment(alignment);
        cursor.mergeBlockFormat(blockFormat);
    }

    letterCounter = ui->textEdit->toPlainText().size();
    qDebug() << "letter cnt : = "<< letterCounter;

    /* cursore */
    // riempio la lista indicizzata

    for(int utente : id_pos.keys()){
        id_colore_cursore.append(qMakePair(qMakePair(utente,id_colore.value(utente)),id_pos.value(utente)));
    }

    std::sort(id_colore_cursore.begin(), id_colore_cursore.end(), sorting);

    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
}

void MainWindow::destroyMain(QString filenam){
    emit openThisFile(filenam);
    MainWindow* mainwindow = new MainWindow(this->socket, nullptr, filenam);
    hide();
    mainwindow->show();
    this->deleteLater();
}

void MainWindow::destroyMainC(QString filename){
    emit newFile(filename);
    MainWindow* mainwindow = new MainWindow(this->socket, nullptr, filename);
    mainwindow->show();
    this->deleteLater();
}

void MainWindow::changeViewAfterInsert(QChar l, int pos, QTextCharFormat format, Qt::AlignmentFlag alignment, QString letterID)
{
    disconnect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
    /*disconnect( socket, SIGNAL(userCursor(QPair<int,int>,QColor)),this, SLOT(on_cursor_triggered(QPair<int,int>,QColor)));
    disconnect( socket, SIGNAL(userCursor(QPair<int,int>,QColor)),
             this, SLOT(on_cursor_triggered(QPair<int,int>,QColor)));
    disconnect( this, SIGNAL(sendCursorChange(int)),
             fHandler, SLOT(localCursorChange(int)));
    disconnect( this, SIGNAL(sendCursorSelection(int,int)),
             socket, SLOT(sendCursorSelectionToServer(int,int)));*/
    QTextCursor cursor(ui->textEdit->textCursor());
    auto vett = this->fHandler->getVectorFile();
    if(pos<vett.size() && vett[pos]->getLetterID().compare(letterID) !=0){
        for (int i = vett.size()-1; i >= 0; i--){
            if(vett[i]->getLetterID().compare(letterID) == 0){
                pos = i;
                break;
            }
        }
    }

    //int prevPos = cursor.position();
    int oldPos = cursor.position();

    cursor.setPosition(pos);
    //ui->textEdit->setAlignment(alignment);
    cursor.insertText(l, format);
    QTextBlockFormat blockFormat = cursor.blockFormat();
    blockFormat.setAlignment(alignment);
    cursor.mergeBlockFormat(blockFormat);
    if(oldPos <= pos){
        cursor.setPosition(oldPos);
    }else{
       cursor.setPosition(oldPos+1);
    }
    ui->textEdit->setTextCursor(cursor);
    qDebug() <<"here";
    qDebug() << cursor.block().text();
    qDebug() << ui->textEdit->alignment();
    //cursor.setPosition(prevPos);
    letterCounter++;

    //CONTROLLO SE ARRIVA IL FORMATO GIUSTO

    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
}

void MainWindow::changeViewAfterDelete(int pos)
{
    if(letterCounter >= 1){
    disconnect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));

        QTextCursor cursor(ui->textEdit->textCursor());

        cursor.setPosition(pos);
        cursor.deletePreviousChar();

        qDebug() << "Devo cancellare la lettera in pos: " << pos;


    letterCounter--;
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
    }
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
            QTextBlockFormat blockFormat = cursor.blockFormat();
            blockFormat.setAlignment(l->getAlignment());
            cursor.mergeBlockFormat(blockFormat);
            ui->textEdit->setTextCursor(cursor);
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
        ui->l_user1->setText(username);
        ui->l_user1->show();
    }

    else if(numberUsersOnline == 2){  //Personalizzo ed accendo la label user2
        ui->user2->setStyleSheet(styleSheet.arg(color.name()));
        ui->user2->setText(username.at(0).toUpper());
        ui->user2->show();
        ui->l_user2->setText(username);
        ui->l_user2->show();
    }

    else if(numberUsersOnline == 3){  //Personalizzo ed accendo la label user3
        ui->user3->setStyleSheet(styleSheet.arg(color.name()));
        ui->user3->setText(username.at(0).toUpper());
        ui->user3->show();
        ui->l_user3->setText(username);
        ui->l_user3->show();
    }

    else {  //Incrementare il contatore
        ui->counter->setText("+" + QString::number(numberUsersOnline-3));
        ui->counter->show();
    }

    /*La lista completa degli Online Users la inizializzo nel OnlineUser Constructor*/

}

void MainWindow::removeUserDisconnect(QString name, int userID){

    int numberUsersOnline = socket->getUserColor().size();

    if(numberUsersOnline == 0){  //Spengo la label user1
        ui->user1->hide();
        ui->l_user1->hide();
    }

    else if(numberUsersOnline == 1){  //Spengo la label user2
        if(ui->l_user1->text() == name){
            ui->user1->setText(ui->user2->text());
            ui->l_user1->setText(ui->l_user2->text());
        }

        ui->user2->hide();
        ui->l_user2->hide();
    }

    else if(numberUsersOnline == 2){  //Spengo la label user3
        if(ui->l_user2->text() == name){
            ui->user2->setText(ui->user3->text());
            ui->l_user2->setText(ui->l_user3->text());
        }

        else if(ui->l_user1->text() == name){
            ui->user1->setText(ui->user2->text());
            ui->user2->setText(ui->user3->text());
            ui->l_user1->setText(ui->l_user2->text());
            ui->l_user2->setText(ui->l_user3->text());
        }

        ui->user3->hide();
        ui->l_user3->hide();
    }

    else {
        ui->counter->hide();
    }


    /*La lista completa degli Online Users la inizializzo nel OnlineUser Constructor*/
}

void MainWindow::on_textEdit_cursorPositionChanged() {
    disconnect(ui->mainToolBar->widgetForAction(ui->mainToolBar->actions().at(7)), SIGNAL(currentFontChanged(QFont)),
               this, SLOT(currentFontChanged(QFont)));
    disconnect(ui->mainToolBar->widgetForAction(ui->mainToolBar->actions().at(8)), SIGNAL(currentIndexChanged(int)),
               this, SLOT(fontSizeChanged(int)));

    /*Questa funzione gestirà la vista dei bottoni dello stile, ovvero se si vedrenno accessi o spenti. */

    QTextCursor cursor(ui->textEdit->textCursor());
    int pos = cursor.position();
    // emit segnale per notificare altri utenti del cambiamento
    if(!cursor.hasSelection() && pos <= ui->textEdit->toPlainText().size() && ui->textEdit->toPlainText().size() <= letterCounter)
        emit sendCursorChange(pos);

    int start = cursor.selectionStart();
    int end = cursor.selectionEnd()-1;

    /*Se il testo selezionato ha stile misto, i bottoni accendono lo stile*/
    if(cursor.hasSelection()==true){
        if(isSelectionBold(start, end+1)){
            ui->actionBold->setChecked(true);
        }
        else {
            ui->actionBold->setChecked(false);
        }

        if(isSelectionItalic(start, end+1)){
            ui->actionItalic->setChecked(true);
        }
        else {
            ui->actionItalic->setChecked(false);
        }

        if(isSelectionUnderlined(start, end+1)){
            ui->actionUnderlined->setChecked(true);
        }
        else {
            ui->actionUnderlined->setChecked(false);
        }

        if(cursor.selectionStart()==0){
            this->firstLetter=ui->textEdit->currentCharFormat();
        }

        auto currFont = ui->textEdit->currentCharFormat().font();
        emit setCurrFont(currFont);
        emit setCurrFontSize(currFont.pointSize()-1);
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

        if(cursor.position()==1){
            this->firstLetter=ui->textEdit->currentCharFormat();
        }

        auto currFont = ui->textEdit->currentCharFormat().font();
        emit setCurrFont(currFont);
        emit setCurrFontSize(currFont.pointSize()-1);
    }

    /* Controllo l'allineamento */
    auto align = ui->textEdit->alignment();

    if(align == Qt::AlignLeft){
        ui->actionAlign_to_Left->setChecked(true);
        ui->actionAlign_to_Right->setChecked(false);
        ui->actionAlign_to_Center->setChecked(false);
        ui->actionAlign_to_Justify->setChecked(false);
    }
    else if(align == Qt::AlignRight){
        ui->actionAlign_to_Left->setChecked(false);
        ui->actionAlign_to_Right->setChecked(true);
        ui->actionAlign_to_Center->setChecked(false);
        ui->actionAlign_to_Justify->setChecked(false);
    }
    else if(align == Qt::AlignCenter){
        ui->actionAlign_to_Left->setChecked(false);
        ui->actionAlign_to_Right->setChecked(false);
        ui->actionAlign_to_Center->setChecked(true);
        ui->actionAlign_to_Justify->setChecked(false);
    }
    else if(align == Qt::AlignJustify){
        ui->actionAlign_to_Left->setChecked(false);
        ui->actionAlign_to_Right->setChecked(false);
        ui->actionAlign_to_Center->setChecked(false);
        ui->actionAlign_to_Justify->setChecked(true);
    }

    connect(ui->mainToolBar->widgetForAction(ui->mainToolBar->actions().at(7)), SIGNAL(currentFontChanged(QFont)),
            this, SLOT(currentFontChanged(QFont)));
    connect(ui->mainToolBar->widgetForAction(ui->mainToolBar->actions().at(8)), SIGNAL(currentIndexChanged(int)),
            this, SLOT(fontSizeChanged(int)));

}

void MainWindow::changeClientImage(QString imageName){

    if(imageName != "") {
        QPixmap userPixmap = QPixmap(imageName);

        if(userPixmap != QPixmap()){
            QPixmap scaled = userPixmap.scaled(ui->myicon->width(), ui->myicon->height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            ui->myicon->setPixmap(scaled);
        }

        else ui->myicon->setText(socket->getClientUsername().at(0).toUpper());
    }

    else ui->myicon->setText(socket->getClientUsername().at(0).toUpper());

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


void MainWindow::on_actionExport_as_PDF_triggered()
{
    QTextDocument document;
    QTextCharFormat fmt;
    fmt.setBackground(Qt::white);
    QTextEdit *t = ui->textEdit;
    QTextCursor cursor = t->textCursor();
    cursor.setPosition(0);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.mergeCharFormat(fmt);
    document.setHtml(ui->textEdit->toHtml());
    QString fn = QFileDialog::getSaveFileName(this, tr("Select output file"), QString(), tr("PDF Files(*.pdf)"));
      if (fn.isEmpty())
        return;
    QPrinter printer;
    printer.setPageMargins(10.0,10.0,10.0,10.0,printer.Millimeter);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setColorMode(QPrinter::Color);
    printer.setOutputFileName(fn);
    document.print(&printer);

    //ripristino cursore
    QTextCharFormat fmt2;
    fmt2.setBackground(Qt::white);
    QColor colore = id_colore_cursore.value(0).first.second;
    int pos = id_colore_cursore.value(0).second;
    qDebug() << pos << id_colore_cursore.size() << id_colore_cursore.value(0);

    fmt.setBackground(colore);

    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
    //qDebug() << "testo from Start: " << cursor.selectedText();
    cursor.mergeCharFormat(fmt2);
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    //qDebug() << "testo to End: " << cursor.selectedText();
    cursor.mergeCharFormat(fmt2);
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
    //qDebug() << "testo left: " << cursor.selectedText();
    cursor.mergeCharFormat(fmt);

    for(int i = 1; i < id_colore_cursore.size(); i++){
        QColor colore = id_colore_cursore.value(i).first.second;
        int pos = id_colore_cursore.value(i).second;

        fmt.setBackground(colore);

        cursor.setPosition(pos);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.mergeCharFormat(fmt2);
        cursor.setPosition(pos);
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
        cursor.mergeCharFormat(fmt);
    }

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
    if(ui->textEdit->toPlainText().length() < 1){
        ui->textEdit->setAlignment(Qt::AlignLeft);
        ui->actionAlign_to_Left->setChecked(true);
        ui->actionAlign_to_Right->setChecked(false);
        ui->actionAlign_to_Center->setChecked(false);
        ui->actionAlign_to_Justify->setChecked(false);
        return;
    }

    disconnect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
              fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
    disconnect(this, SIGNAL(myDelete(int,int)),
              fHandler, SLOT(localDelete(int,int)));

    ui->textEdit->setAlignment(Qt::AlignLeft);
    ui->actionAlign_to_Left->setChecked(true);
    ui->actionAlign_to_Right->setChecked(false);
    ui->actionAlign_to_Center->setChecked(false);
    ui->actionAlign_to_Justify->setChecked(false);

    QTextCursor cursor = ui->textEdit->textCursor();
    int endSelection = cursor.selectionEnd();

    QVector<Letter*> file = this->fHandler->getVectorFile();
    QTextBlock paragraph = cursor.block();
    int startIndex, length, lastIndex;
    QString startID, lastID;
    bool continueLoop = true;

    while(continueLoop) {
        /* Get startID and lastID of current paragraph */
        startIndex = paragraph.position();
        length = paragraph.length();
        lastIndex = startIndex + length - 1;
        if(length == 1) { // paragrafo vuoto
            startID = "-1";
            lastID = "-1";
        } else {
            startID = file.at(startIndex)->getLetterID();
            if(file.size() > lastIndex) // file vector includes \n of that paragraph
                lastID = file.at(lastIndex)->getLetterID();
            else if(file.size() == lastIndex)
                lastID = file.at(lastIndex-1)->getLetterID();
        }
        emit sendAlignment(Qt::AlignLeft, startIndex, startID, lastID);

        if(paragraph.contains(endSelection))
            continueLoop = false;
        else paragraph = paragraph.next();
    }

    qDebug() << ui->textEdit->alignment() << Qt::AlignLeft;
    connect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
              fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
    connect(this, SIGNAL(myDelete(int,int)),
              fHandler, SLOT(localDelete(int,int)));
}

void MainWindow::on_actionAlign_to_Right_triggered()
{
    if(ui->textEdit->toPlainText().length() < 1){
        ui->textEdit->setAlignment(Qt::AlignRight);
        ui->actionAlign_to_Left->setChecked(false);
        ui->actionAlign_to_Right->setChecked(true);
        ui->actionAlign_to_Center->setChecked(false);
        ui->actionAlign_to_Justify->setChecked(false);
        return;
    }

    disconnect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
              fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
    disconnect(this, SIGNAL(myDelete(int,int)),
              fHandler, SLOT(localDelete(int,int)));

    ui->textEdit->setAlignment(Qt::AlignRight);
    ui->actionAlign_to_Left->setChecked(false);
    ui->actionAlign_to_Right->setChecked(true);
    ui->actionAlign_to_Center->setChecked(false);
    ui->actionAlign_to_Justify->setChecked(false);

    QTextCursor cursor = ui->textEdit->textCursor();
    int endSelection = cursor.selectionEnd();

    QVector<Letter*> file = this->fHandler->getVectorFile();
    QTextBlock paragraph = cursor.block();
    int startIndex, length, lastIndex;
    QString startID, lastID;
    bool continueLoop = true;

    while(continueLoop) {
        /* Get startID and lastID of current paragraph */
        startIndex = paragraph.position();
        length = paragraph.length();
        lastIndex = startIndex + length - 1;
        if(length == 1) { // paragrafo vuoto
            startID = "-1";
            lastID = "-1";
        } else {
            startID = file.at(startIndex)->getLetterID();
            if(file.size() > lastIndex) // file vector includes \n of that paragraph
                lastID = file.at(lastIndex)->getLetterID();
            else if(file.size() == lastIndex)
                lastID = file.at(lastIndex-1)->getLetterID();
        }
        emit sendAlignment(Qt::AlignRight, startIndex, startID, lastID);

        if(paragraph.contains(endSelection))
            continueLoop = false;
        else paragraph = paragraph.next();
        qDebug() << continueLoop;
    }

    connect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
              fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
    connect(this, SIGNAL(myDelete(int,int)),
              fHandler, SLOT(localDelete(int,int)));
}

void MainWindow::on_actionAlign_to_Center_triggered()
{
    if(ui->textEdit->toPlainText().length() < 1){
        ui->textEdit->setAlignment(Qt::AlignCenter);
        ui->actionAlign_to_Left->setChecked(false);
        ui->actionAlign_to_Right->setChecked(false);
        ui->actionAlign_to_Center->setChecked(true);
        ui->actionAlign_to_Justify->setChecked(false);
        return;
    }

    disconnect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
              fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
    disconnect(this, SIGNAL(myDelete(int,int)),
              fHandler, SLOT(localDelete(int,int)));

    ui->textEdit->setAlignment(Qt::AlignCenter);
    ui->actionAlign_to_Left->setChecked(false);
    ui->actionAlign_to_Right->setChecked(false);
    ui->actionAlign_to_Center->setChecked(true);
    ui->actionAlign_to_Justify->setChecked(false);

    QTextCursor cursor = ui->textEdit->textCursor();
    int endSelection = cursor.selectionEnd();

    QVector<Letter*> file = this->fHandler->getVectorFile();
    QTextBlock paragraph = cursor.block();
    int startIndex, length, lastIndex;
    QString startID, lastID;
    bool continueLoop = true;

    while(continueLoop) {
        /* Get startID and lastID of current paragraph */
        startIndex = paragraph.position();
        length = paragraph.length();
        lastIndex = startIndex + length - 1;
        if(length == 1) { // paragrafo vuoto
            startID = "-1";
            lastID = "-1";
        } else {
            startID = file.at(startIndex)->getLetterID();
            if(file.size() > lastIndex) // file vector includes \n of that paragraph
                lastID = file.at(lastIndex)->getLetterID();
            else if(file.size() == lastIndex)
                lastID = file.at(lastIndex-1)->getLetterID();
        }
        emit sendAlignment(Qt::AlignCenter, startIndex, startID, lastID);

        if(paragraph.contains(endSelection))
            continueLoop = false;
        else paragraph = paragraph.next();
        qDebug() << continueLoop;
    }

    connect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
              fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
    connect(this, SIGNAL(myDelete(int,int)),
              fHandler, SLOT(localDelete(int,int)));
}

void MainWindow::on_actionAlign_to_Justify_triggered()
{
    if(ui->textEdit->toPlainText().length() < 1){
        ui->textEdit->setAlignment(Qt::AlignJustify);
        ui->actionAlign_to_Left->setChecked(false);
        ui->actionAlign_to_Right->setChecked(false);
        ui->actionAlign_to_Center->setChecked(false);
        ui->actionAlign_to_Justify->setChecked(true);
        return;
    }

    disconnect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
              fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
    disconnect(this, SIGNAL(myDelete(int,int)),
              fHandler, SLOT(localDelete(int,int)));

    ui->textEdit->setAlignment(Qt::AlignJustify);
    ui->actionAlign_to_Left->setChecked(false);
    ui->actionAlign_to_Right->setChecked(false);
    ui->actionAlign_to_Center->setChecked(false);
    ui->actionAlign_to_Justify->setChecked(true);

    QTextCursor cursor = ui->textEdit->textCursor();
    int endSelection = cursor.selectionEnd();

    QVector<Letter*> file = this->fHandler->getVectorFile();
    QTextBlock paragraph = cursor.block();
    int startIndex, length, lastIndex;
    QString startID, lastID;
    bool continueLoop = true;

    while(continueLoop) {
        /* Get startID and lastID of current paragraph */
        startIndex = paragraph.position();
        length = paragraph.length();
        lastIndex = startIndex + length - 1;
        if(length == 1) { // paragrafo vuoto
            startID = "-1";
            lastID = "-1";
        } else {
            startID = file.at(startIndex)->getLetterID();
            if(file.size() > lastIndex) // file vector includes \n of that paragraph
                lastID = file.at(lastIndex)->getLetterID();
            else if(file.size() == lastIndex)
                lastID = file.at(lastIndex-1)->getLetterID();
        }
        emit sendAlignment(Qt::AlignJustify, startIndex, startID, lastID);

        if(paragraph.contains(endSelection))
            continueLoop = false;
        else paragraph = paragraph.next();
        qDebug() << continueLoop;
    }

    connect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
              fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
    connect(this, SIGNAL(myDelete(int,int)),
              fHandler, SLOT(localDelete(int,int)));
}

void MainWindow::notConnected(){
    serverDisc *s = new serverDisc(this);
    s->show();
    this->setWindowState(Qt::WindowState::WindowActive);
}


void MainWindow::on_cursor_triggered(QPair<int,int> idpos, QColor col, QString letterID)
{
    // controllo che nella mappa colorecursore non sia gia presente l'id

    disconnect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));

    QTextCharFormat fmt;
    QTextCharFormat fmt2;

    fmt2.setBackground(Qt::white);

    QTextCursor cursor = ui->textEdit->textCursor();

    // controllo che nella mappa colore-cursore non sia gia presente il colore
    bool trovato = false;
    for(int i = 0; i< id_colore_cursore.size(); i++){
        //se c'è lo sostituisco
        if (id_colore_cursore.at(i).first.first == idpos.first){
            if(idpos.second >= 0){
                id_colore_cursore.replace(i, qMakePair(qMakePair(idpos.first,col), idpos.second));
                trovato = true;
                break;
            }
            else{
                id_colore_cursore.removeAt(i);
                trovato = true;
                break;
            }
        }

    }

    //altrimenti lo aggiungo
    if (trovato == false)
        id_colore_cursore.append(qMakePair(qMakePair(idpos.first,col), idpos.second));

    std::sort(id_colore_cursore.begin(), id_colore_cursore.end(), sorting);
    auto vettore = this->fHandler->getVectorFile();
    QColor colore = id_colore_cursore.value(0).first.second;
    int pos = id_colore_cursore.value(0).second;

    if (idpos.second < 0) {
        pos = 0;
        colore = Qt::white;
    }
    else if (idpos.second > ui->textEdit->toPlainText().size())
        pos = letterCounter;

    qDebug() << pos << id_colore_cursore.size() << id_colore_cursore.value(0);

    fmt.setBackground(colore);

    if((pos == 0 && pos <= ui->textEdit->toPlainText().size()))
    {
        cursor.setPosition(pos);
        cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
        //qDebug() << "testo from Start: " << cursor.selectedText();
        cursor.mergeCharFormat(fmt2);
        cursor.setPosition(pos);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        //qDebug() << "testo to End: " << cursor.selectedText();
        cursor.mergeCharFormat(fmt2);
        cursor.setPosition(pos);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        //qDebug() << "testo left: " << cursor.selectedText();
        cursor.mergeCharFormat(fmt);
    }
    else if(pos <= ui->textEdit->toPlainText().size() && (vettore.at(pos-1)->getValue() == "\n")){
        cursor.setPosition(pos);
        cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
        //qDebug() << "testo from Start: " << cursor.selectedText();
        cursor.mergeCharFormat(fmt2);
        cursor.setPosition(pos);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        //qDebug() << "testo to End: " << cursor.selectedText();
        cursor.mergeCharFormat(fmt2);
        cursor.setPosition(pos);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        //qDebug() << "testo left: " << cursor.selectedText();
        cursor.mergeCharFormat(fmt);
    }

    else{
        cursor.setPosition(pos);
                cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
                //qDebug() << "testo from Start: " << cursor.selectedText();
                cursor.mergeCharFormat(fmt2);
                cursor.setPosition(pos);
                cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
                //qDebug() << "testo to End: " << cursor.selectedText();
                cursor.mergeCharFormat(fmt2);
                cursor.setPosition(pos);
                cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
                //qDebug() << "testo left: " << cursor.selectedText();
                cursor.mergeCharFormat(fmt);
    }


    for(int i = 1; i < id_colore_cursore.size(); i++){
        QColor colore = id_colore_cursore.value(i).first.second;
        int pos = id_colore_cursore.value(i).second;

        fmt.setBackground(colore);

        if((pos == 0 || (vettore.at(pos-1)->getValue() == "\n")) && pos <= ui->textEdit->toPlainText().size())
        {
            cursor.setPosition(pos);
            cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            //qDebug() << "testo to End: " << cursor.selectedText();
            cursor.mergeCharFormat(fmt2);
            cursor.setPosition(pos);
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
            //qDebug() << "testo left: " << cursor.selectedText();
            cursor.mergeCharFormat(fmt);
        }
        else{
            cursor.setPosition(pos);
            cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            cursor.mergeCharFormat(fmt2);
            cursor.setPosition(pos);
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
            cursor.mergeCharFormat(fmt);
        }
    }
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
}

void MainWindow::currentFontChanged(QFont font){
    auto cursor = ui->textEdit->textCursor();

    /* CASO1: Non sto selezionando niente */
    if(cursor.selectionStart() - cursor.selectionEnd() == 0){
        auto currFont = ui->textEdit->currentCharFormat();
        currFont.setFont(font, QTextCharFormat::FontPropertiesSpecifiedOnly);
        ui->textEdit->mergeCurrentCharFormat(currFont);
    }

    /*CASO2: Cambio il font di una selezione*/
    else{
        disconnect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
        disconnect(this, SIGNAL(myDelete(int,int)),
                  fHandler, SLOT(localDelete(int,int)));

        QTextCharFormat currFont = ui->textEdit->currentCharFormat();
        qDebug() << "CHANGE" << font << currFont.font();
        currFont.setFont(font, QTextCharFormat::FontPropertiesSpecifiedOnly);
        ui->textEdit->mergeCurrentCharFormat(currFont);

        /* Aggiorno il modello */
        QMap<QString, QTextCharFormat> formatCharMap;
        auto vettore = this->fHandler->getVectorFile();
        int i=0;

        int start = cursor.selectionStart();
        int end = cursor.selectionEnd()-1;

        QString startID = vettore.at(start)->getLetterID();
        QString lastID = vettore.at(end)->getLetterID();

        for(i=start; i<=end; i++){
            /* Se testo selezionato misto, allora settare, altrimenti se tutto settato, togliere */
            cursor.setPosition(i+1);
            auto letterFormat = cursor.charFormat();
            formatCharMap.insert(vettore.at(i)->getLetterID(), letterFormat);
        }

        emit styleChange(formatCharMap);

        connect( this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
        connect( this, SIGNAL(myDelete(int,int)),
                  fHandler, SLOT(localDelete(int,int)));
    }
}

void MainWindow::fontSizeChanged(int size){
    int fontSize = size+1;
    auto cursor = ui->textEdit->textCursor();

    /* CASO1: Non sto selezionando niente */
    if(cursor.selectionStart() - cursor.selectionEnd() == 0){
        auto currFont = ui->textEdit->currentCharFormat();
        currFont.setFontPointSize(fontSize);
        ui->textEdit->mergeCurrentCharFormat(currFont);
    }

    /*CASO2: Cambio il font di una selezione*/
    else{
        disconnect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
        disconnect(this, SIGNAL(myDelete(int,int)),
                  fHandler, SLOT(localDelete(int,int)));

        QTextCharFormat currFont = ui->textEdit->currentCharFormat();
        currFont.setFontPointSize(fontSize);
        ui->textEdit->mergeCurrentCharFormat(currFont);

        /* Aggiorno il modello */
        QMap<QString, QTextCharFormat> formatCharMap;
        auto vettore = this->fHandler->getVectorFile();
        int i=0;

        int start = cursor.selectionStart();
        int end = cursor.selectionEnd()-1;

        QString startID = vettore.at(start)->getLetterID();
        QString lastID = vettore.at(end)->getLetterID();

        for(i=start; i<=end; i++){
            /* Se testo selezionato misto, allora settare, altrimenti se tutto settato, togliere */
            cursor.setPosition(i+1);
            auto letterFormat = cursor.charFormat();
            formatCharMap.insert(vettore.at(i)->getLetterID(), letterFormat);
        }

        emit styleChange(formatCharMap);

        connect( this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
                  fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
        connect( this, SIGNAL(myDelete(int,int)),
                  fHandler, SLOT(localDelete(int,int)));
    }
}


void MainWindow::on_actionhistory_triggered()
{
    emit sendHist();
}

void MainWindow::uploadHistory(QMap<int, QString> mapIdUsername){
    usersLettersWindow* history = new usersLettersWindow(mapIdUsername, fHandler->getVectorFile(), this);
    history->show();
}


void MainWindow::on_actionLight_triggered()
{
    pal.setColor(QPalette::Background, Qt::white);
    pal.setColor(QPalette::WindowText, Qt::black);
    pal.setColor(QPalette::ButtonText, Qt::black);
    ui->username->setStyleSheet("color:black");
    ui->l_user1->setStyleSheet("color:black");
    ui->l_user2->setStyleSheet("color:black");
    ui->l_user3->setStyleSheet("color:black");
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->show();
}

void MainWindow::changeViewAfterColor(int start, int end, QColor colore){

    disconnect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));

    QTextCharFormat fmt;
    QTextCursor cursor = ui->textEdit->textCursor();

    fmt.setForeground(colore);
    int dif = end-start+1;
    cursor.setPosition(end+1);
    for(int i = 0; i<=dif; i++){
        cursor.mergeCharFormat(fmt);
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
         qDebug() << "testo left: " << cursor.selectedText();
    }
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
}


void MainWindow::on_actionDark_triggered()
{
    pal.setColor(QPalette::Background, QColor(58,58,60));
    pal.setColor(QPalette::WindowText, Qt::white);
    pal.setColor(QPalette::ButtonText, Qt::white);
    ui->username->setStyleSheet("color:white");
    ui->l_user1->setStyleSheet("color:white");
    ui->l_user2->setStyleSheet("color:white");
    ui->l_user3->setStyleSheet("color:white");
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->show();
}

void MainWindow::changeAlignment(Qt::AlignmentFlag alignment, int cursorPosition){
    disconnect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
              fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
    disconnect(this, SIGNAL(myDelete(int,int)),
              fHandler, SLOT(localDelete(int,int)));

    QTextCursor cursor = ui->textEdit->textCursor();
    int prevPos = cursor.position();
    cursor.setPosition(cursorPosition);
    QTextBlockFormat blockFormat = cursor.blockFormat();
    blockFormat.setAlignment(alignment);
    cursor.mergeBlockFormat(blockFormat);
    cursor.setPosition(prevPos);
    ui->textEdit->setTextCursor(cursor);
    qDebug() << cursor.block().text();

    connect(this, SIGNAL(myInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)),
              fHandler, SLOT(localInsert(int, QChar, int, QTextCharFormat, Qt::AlignmentFlag)));
    connect(this, SIGNAL(myDelete(int,int)),
              fHandler, SLOT(localDelete(int,int)));

}

/*void MainWindow::on_textEdit_selectionChanged()
{
    QTextCursor cursor = ui->textEdit->textCursor();
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    if(start == end) {
       // emit sendCursorChange(start);
        return;
    }
    qDebug() << "testo selezionato: " << cursor.selectedText();
    emit sendCursorSelection(start, end);
}

void MainWindow::changeViewAfterSelection(int start, int end, QColor colore)
{
    disconnect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));

    QTextCharFormat fmt, fmt2;
    QTextCursor cursor = ui->textEdit->textCursor();
    fmt.setBackground(colore);
    if(start > 0) {
        fmt2.setBackground(Qt::white);
        cursor.setPosition(start-1);
        cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
        cursor.mergeCharFormat(fmt2);
        cursor.setPosition(start-1);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.mergeCharFormat(fmt2);
    }
    cursor.setPosition(end);
    for(int i = start; i<=end; i++){
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
        cursor.mergeCharFormat(fmt);
         qDebug() << "testo left: " << cursor.selectedText();
    }
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
}*/

void MainWindow::insertPastedText(QString html, QString text){
    disconnect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));

    QTextCursor cursor(ui->textEdit->textCursor());
    int externalIndex = cursor.position();

    if(cursor.hasSelection()){
        int deletedLetters = cursor.selectionEnd()-cursor.selectionStart();
        if (receivers(SIGNAL(myDelete(int,int))) > 0) {
            letterCounter -= deletedLetters;
            externalIndex = cursor.selectionStart();
            emit myDelete(externalIndex+1, cursor.selectionEnd());
            cursor.setPosition(cursor.selectionStart());
        }
    }
    qDebug() << html;
    ui->textEdit->insertHtml(html);
    QTextCharFormat fmt;

    if (receivers(SIGNAL(myInsert(int,QChar,int,QTextCharFormat,Qt::AlignmentFlag))) > 0) {
        for(int i = 0; i < text.size(); i++){
            letterCounter++;
            externalIndex++;
            cursor.setPosition(externalIndex);

            if(cursor.charFormat().background() != Qt::white){
                fmt = cursor.charFormat();
                fmt.setBackground(Qt::white);
                cursor.setPosition(externalIndex);
                cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
                cursor.mergeCharFormat(fmt);

            }

            QTextBlockFormat block = cursor.blockFormat();
            qDebug() << "CHAR FORMAT" << block.alignment();
            QThread::msleep(1);
            emit myInsert(externalIndex, text.at(i), socket->getClientID(), cursor.charFormat(), this->getFlag(block.alignment()));
        }
        emit sendCursorChange(externalIndex);
    }

    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
}


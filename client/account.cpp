#include "account.h"
#include "ui_account.h"
#include "mainwindow.h"
#include <QTimer>

Account::Account(Socket *sock, QWidget *parent, QString name) :
    QDialog(parent),
    ui(new Ui::Account),
    socket(sock),
    name(name),
    saveChanges(false)
{
    ui->setupUi(this);
    setWindowTitle("Edit Profile");
    QPalette pal = palette();

    /* Set black background */
    pal.setColor(QPalette::Background, QColor(58,58,60));
    pal.setColor(QPalette::WindowText, Qt::white);
    pal.setColor(QPalette::Button, QColor(229,229,234));
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    QPalette p = ui->lineEdit->palette();
    p.setColor(QPalette::Base, QColor(209,209,214));
    p.setColor(QPalette::Text, Qt::black);
    ui->lineEdit->setPalette(p);
    ui->lineEdit_1->setPalette(p);

    ui->lineEdit->setStyleSheet(":focus {border: 2px solid #ace4c6};");
    ui->lineEdit->setAttribute(Qt::WA_MacShowFocusRect,0);
    ui->lineEdit_1->setStyleSheet(":focus {border: 2px solid #ace4c6};");
    ui->lineEdit_1->setAttribute(Qt::WA_MacShowFocusRect,0);
    ui->pushButton->setStyleSheet("color: white; background-color: #706d82;  border-radius:5px");

    /* Camera Button */
    QIcon *user_icon= new QIcon(":/rec/icone/camera_icon.png");
    ui->setImage->setIcon(*user_icon);
    ui->setImage->setIconSize(QSize(18, 18));

    QString styleSheet = "QPushButton {background-color: white; border-style: solid; border-width: 1px; border-radius: 15px; border-color: rgb(0, 0, 0);} QPushButton:hover {background-color: rgb(233, 233, 233)} QPushButton:pressed {background-color: rgb(181, 181, 181)}";
    ui->setImage->setStyleSheet(styleSheet);

    /* Imposto la User Image e nome utente*/
    ui->username->setText(socket->getClientUsername());

    styleSheet = "QLabel { background-color: rgb(255, 252, 247); color: black; border-style: solid; border-width: 2px; border-radius: 6px; border-color: orange; font: ; }";
    ui->userImage->setStyleSheet(styleSheet);
    QFont font("Arial", 50);
    ui->userImage->setFont(font);
    QString imageName = QString::number(socket->getClientID())+".png";
    qDebug() << socket->getClientID();
    QPixmap userPixmap = QPixmap(imageName);


    if(userPixmap != QPixmap()){
        oldImage = imageName;
        QPixmap scaled = userPixmap.scaled(ui->userImage->width(), ui->userImage->height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        ui->userImage->setPixmap(scaled);
    }

    else {
        oldImage = "";
        ui->userImage->setText(socket->getClientUsername().at(0).toUpper()); //Si dovrebbe impostare la propria iniziale
    }

    /* Connect */
    connect( this, SIGNAL(updateClientImageOnMainWindow(QString)),
             parent, SLOT(changeClientImage(QString)));
    connect( this, SIGNAL(updateInfoClient(QString, QString)),
             sock, SLOT(sendChange(QString, QString)));

    this->show();
    QTimer::singleShot(0, ui->lineEdit, SLOT(setFocus()));
}

Account::~Account()
{
    delete ui;
}

void Account::closeEvent(QCloseEvent *event){

    if(!this->saveChanges){
        /* Se non ho salvato le modifiche, ripristino la vecchia userImage */
        emit updateClientImageOnMainWindow(oldImage);
    }

    event->accept();
}

void Account::on_pushButton_clicked()
{
    // Notificare al server i cambiamenti
    QString new_password = ui->lineEdit->text();
    QString confirm_password = ui->lineEdit_1->text();

    if(QString::compare(new_password, confirm_password) == 0){
        qDebug() <<  this->imageChanged;
        if(this->imageChanged){
            /* Se l'immagine è stata cambiata invio il nuovo path che può essere:
               - "" se l'utente ha deciso di eliminare l'immagine;
               - "pathImage" se l'utente ha selezionato una nuova immagine;  */
            QString nameNewImage = QString::number(socket->getClientID())+".png";
            if(QString::compare(this->pathUserImage, "") == 0 && QFile::exists(nameNewImage)){
                /* C'era un'immagine ed è stata tolta */
                //QFile::remove(nameNewImage);
            }
            else if(QFile::exists(nameNewImage)) {
                /* C'era un'immagine ed è stata aggiornata */
                QFile::remove(nameNewImage);
                QFile::copy(this->pathUserImage, nameNewImage);
            }
            /* Non c'era un'immagine ed è stata aggiunta */
            else QFile::copy(this->pathUserImage, nameNewImage);
            emit updateInfoClient(new_password, this->pathUserImage);
        }

        /* Se l'immagine non è stata cambiata invio la stringa "notChanged" */
        else emit updateInfoClient(new_password, "null");

        this->saveChanges = true;
        this->close();
    }

    else QMessageBox::warning(this, "Ops...", "Passwords are not equal");
}

void Account::on_setImage_clicked()
{
    QString iconName = QFileDialog::getOpenFileName(this, tr("Choose"), "", tr("Images (*.png *.jpg *.jpeg *.bmp)"));
    if(QString::compare(iconName, QString("")) != 0)
    {
        QPixmap userPixmap = QPixmap(iconName);
        QPixmap scaled = userPixmap.scaled(ui->userImage->width(), ui->userImage->height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        ui->userImage->setPixmap(scaled);
        this->pathUserImage = iconName;
        this->imageChanged = true;
    }
    emit updateClientImageOnMainWindow(iconName);
}

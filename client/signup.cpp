#include "signup.h"
#include "ui_signup.h"
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QBitmap>
#include <QPainter>
#include <QTimer>
#include <QShortcut>

SignUp::SignUp(Socket* sock, QWidget* parent)
    : QDialog(parent),
    ui(new Ui::SignUp),
    socket(sock),
    pathUserImage("1.png")
{
    ui->setupUi(this);
    QPalette pal = palette();
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    this->setWindowFlags(Qt::Window);
    QShortcut *sc = new QShortcut(QKeySequence("Return"),ui->groupBox_2);
    connect(sc, SIGNAL(activated()), ui->pushButton, SLOT(click()));

    // set black background
    pal.setColor(QPalette::Background, QColor(58,58,60));
    pal.setColor(QPalette::WindowText, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    QPalette p = ui->lineEdit->palette();
    p.setColor(QPalette::Base, QColor(209,209,214));
    p.setColor(QPalette::Text, Qt::black);
    ui->lineEdit->setPalette(p);
    ui->lineEdit_2->setPalette(p);
    ui->lineEdit_3->setPalette(p);

    ui->lineEdit->setStyleSheet(":focus {border: 2px solid #ace4c6};");
    ui->lineEdit->setAttribute(Qt::WA_MacShowFocusRect,0);
    ui->lineEdit_2->setStyleSheet(":focus {border: 2px solid #ace4c6};");
    ui->lineEdit_2->setAttribute(Qt::WA_MacShowFocusRect,0);
    ui->lineEdit_3->setStyleSheet(":focus {border: 2px solid #ace4c6};");
    ui->lineEdit_3->setAttribute(Qt::WA_MacShowFocusRect,0);

    QTimer::singleShot(0, ui->lineEdit, SLOT(setFocus()));

    /* User's Image */
    QString styleSheet = "QLabel { background-color: rgb(255, 254, 239); color: black; border-style: solid; border-width: 1.2px; border-color: black;}";
    ui->userImage->setStyleSheet(styleSheet);
    QFont font("Arial", 30);
    ui->userImage->setFont(font);
    ui->userImage->setText("U"); //Si dovrebbe impostare la propria iniziale

    /* Camera Button */
    QIcon *user_icon= new QIcon(":/rec/icone/camera_icon.png");
    ui->setImage->setIcon(*user_icon);
    ui->setImage->setIconSize(QSize(18, 18));

    styleSheet = "QPushButton {background-color: white; border-style: solid; border-width: 1px; border-radius: 15px; border-color: rgb(0, 0, 0);} QPushButton:hover {background-color: rgb(233, 233, 233)} QPushButton:pressed {background-color: rgb(181, 181, 181)}";
    ui->setImage->setStyleSheet(styleSheet);

    /* Discard Button */
    QIcon *discard_icon= new QIcon(":/rec/icone/discard_icon.png");
    ui->discardImage->setIcon(*discard_icon);
    ui->discardImage->setIconSize(QSize(18, 18));

    styleSheet = "QPushButton {background-color: white; border-style: solid; border-width: 1px; border-radius: 15px; border-color: rgb(0, 0, 0);} QPushButton:hover {background-color: rgb(233, 233, 233)} QPushButton:pressed {background-color: rgb(181, 181, 181)}";
    ui->discardImage->setStyleSheet(styleSheet);
    ui->discardImage->hide();

    ui->pushButton->setStyleSheet("color: white; background-color: #706d82;  border-radius:5px");
    ui->pushButton_2->setStyleSheet("color: white; background-color: #706d82;  border-radius:5px");


    this->show();
    setWindowTitle("");
    socket->isSigningUp(true);
    connect(socket, SIGNAL(signUpSuccess()), this, SLOT(sendToLogin()));
    connect(socket, SIGNAL(signUpError()), this, SLOT(repeatSignUp()));
    connect(socket, SIGNAL(invalidUsername()), this, SLOT(changeUsername()));
}

SignUp::~SignUp() {
    delete ui;
}

void SignUp::on_pushButton_clicked() {
    if(QString::compare(ui->lineEdit->text(),"")==0)
        QMessageBox::warning(this, "Ops...", "Insert username");

    else if(QString::compare(ui->lineEdit_2->text(),"")==0&&QString::compare(ui->lineEdit_3->text(),"")==0)
        QMessageBox::warning(this, "Ops...", "Insert password");

    else if(QString::compare(ui->lineEdit_2->text(), ui->lineEdit_3->text()) == 0){
        QString username = ui->lineEdit->text();
        QByteArray password = ui->lineEdit_2->text().toLatin1();
        QString pathImage = this->pathUserImage;

        socket->sendSignUpRequest(username, password, pathImage);
    }

    else QMessageBox::warning(this, "Ops...", "Passwords are not equal");
}

void SignUp::sendToLogin() {
    // Reindirizzare alla pagina di login
    QMessageBox::information(this, "OK!", "Sign up successful", "Log in");
    loginWindow = new Login(socket, this);
    hide();
    loginWindow->show();
}

void SignUp::repeatSignUp() {
    QMessageBox::warning(this, "Ops...", "Sign up failed. Retry!");
}

void SignUp::changeUsername() {
    QMessageBox::warning(this, "Ops...", "Username already in use: pick another one!");
}


void SignUp::on_setImage_clicked()
{
    QString iconName = QFileDialog::getOpenFileName(this, tr("Choose"), "", tr("Images (*.png *.jpg *.jpeg *.bmp)"));
    if(QString::compare(iconName, QString()) != 0)
    {
        QPixmap userPixmap = QPixmap(iconName);
        QPixmap scaled = userPixmap.scaled(ui->userImage->width(), ui->userImage->height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        ui->userImage->setPixmap(scaled);

        ui->discardImage->show();
        this->pathUserImage = iconName;
    }
}

void SignUp::on_lineEdit_textEdited(const QString &presentText)
{
    if(ui->userImage->pixmap() == nullptr){
        if(QString::compare(presentText, QString()) != 0){
            ui->userImage->setText(presentText.at(0).toUpper());
        }

        else ui->userImage->setText("U");
    }

}

void SignUp::on_discardImage_clicked(){
    ui->userImage->clear();
    QString presentText = ui->lineEdit->text();

    if(QString::compare(presentText, QString()) != 0){
        ui->userImage->setText(presentText.at(0).toUpper());
    }

    else ui->userImage->setText("U");

    ui->discardImage->hide();
    this->pathUserImage="";
}

void SignUp::on_pushButton_2_clicked()
{
    loginWindow = new Login(socket, this);
    hide();
    loginWindow->show();
}

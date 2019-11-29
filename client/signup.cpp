#include "signup.h"
#include "ui_signup.h"
#include <QMessageBox>

SignUp::SignUp(Socket* sock, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::SignUp)
    , socket(sock)
{
    //ui->setupUi(this);  ERRORE ?
}

SignUp::~SignUp() {
    delete ui;
}

void SignUp::on_pushButton_clicked() {
    QString username;
    QString password;
    //QString username = ui->lineEdit_username->text();
    //QByteArray password = ui->lineEdit_password->text().toLatin1();

    connect(socket, SIGNAL(signUpSuccess()), this, SLOT(sendToLogin()));
    connect(socket, SIGNAL(signUpError()), this, SLOT(repeatSignUp()));
    connect(socket, SIGNAL(invalidUsername()), this, SLOT(changeUsername()));

    socket->sendSignUpRequest(username, password);
}

void SignUp::sendToLogin() {
    // Reindirizzare alla pagina di login
    loginWindow = new Login(socket, this);
    hide();
    loginWindow->show();
}

void SignUp::repeatSignUp() {
    QMessageBox::warning(this, "Sign up failed", "Retry");
}

void SignUp::changeUsername() {
    QMessageBox::warning(this, "Username already in use", "Retry");
}

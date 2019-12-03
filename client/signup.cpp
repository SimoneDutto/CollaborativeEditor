#include "signup.h"
#include "ui_signup.h"
#include <QMessageBox>

SignUp::SignUp(Socket* sock, QWidget* parent)
    : QDialog(parent),
    ui(new Ui::SignUp),
    socket(sock)
{
    ui->setupUi(this);
    socket->isSigningUp(true);
    connect(socket, SIGNAL(signUpSuccess()), this, SLOT(sendToLogin()));
    connect(socket, SIGNAL(signUpError()), this, SLOT(repeatSignUp()));
    connect(socket, SIGNAL(invalidUsername()), this, SLOT(changeUsername()));
}

SignUp::~SignUp() {
    delete ui;
}

void SignUp::on_pushButton_clicked() {
    QString username = ui->lineEdit->text();
    QByteArray password = ui->lineEdit_2->text().toLatin1();
    QByteArray conferma = ui->lineEdit_3->text().toLatin1();

    socket->sendSignUpRequest(username, password);
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

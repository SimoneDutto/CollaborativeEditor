#include "signup.h"
#include "ui_signup.h"
#include <QMessageBox>

SignUp::SignUp(Socket* sock, QWidget* parent)
    : QDialog(parent),
    ui(new Ui::SignUp),
    socket(sock)
{
    ui->setupUi(this);
}

SignUp::~SignUp() {
    delete ui;
}

void SignUp::on_pushButton_clicked() {
    QString username = ui->lineEdit->text();
    QByteArray password = ui->lineEdit_2->text().toLatin1();
    QByteArray conferma = ui->lineEdit_3->text().toLatin1();

    connect(socket, SIGNAL(signUpSuccess()), this, SLOT(sendToLogin()));
    connect(socket, SIGNAL(signUpError()), this, SLOT(repeatSignUp()));
    connect(socket, SIGNAL(invalidUsername()), this, SLOT(changeUsername()));

    //if(password==conferma)
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

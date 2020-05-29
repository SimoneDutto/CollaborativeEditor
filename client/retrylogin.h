#ifndef RETRYLOGIN_H
#define RETRYLOGIN_H

#include <QDialog>

namespace Ui {
class retryLogin;
}

class retryLogin : public QDialog
{
    Q_OBJECT

public:
    explicit retryLogin(QWidget *parent = nullptr);
    ~retryLogin();

private slots:
    void on_pushButton_clicked();

private:
    Ui::retryLogin *ui;
};

#endif // RETRYLOGIN_H

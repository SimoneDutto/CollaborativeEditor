#ifndef ERROR_H
#define ERROR_H

#include <QDialog>

namespace Ui {
class Error;
}

class Error : public QDialog
{
    Q_OBJECT

public:
    explicit Error(QWidget *parent = nullptr);
    ~Error();

private slots:
    void on_pushButton_clicked();

private:
    Ui::Error *ui;
};

#endif // ERROR_H

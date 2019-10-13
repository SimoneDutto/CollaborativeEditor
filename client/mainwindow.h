#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "socket.h"
#include "dialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Socket *sock = nullptr, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionNew_triggered();

    void on_actionOpen_triggered();

//    void on_actionSave_triggered();

//    void on_actionSave_As_triggered();

    void on_actionCut_triggered();

    void on_actionCopy_triggered();

    void on_actionRedo_triggered();

    void on_actionPaste_triggered();

    void on_actionUndo_triggered();

    void on_actionAbout_us_triggered();

    void on_actionBold_triggered();

    void on_actionItalic_triggered();

    void on_actionUnderlined_triggered();

    void on_actionFont_triggered();

    void on_actionColor_triggered();

    void on_actionBackgorund_Color_triggered();
    
    void on_textEdit_textChanged();

    void on_lineEdit_editingFinished();

private:
    Ui::MainWindow *ui;
    Socket *socket;
    QString file_path;
    Dialog *dialog;

signals:
    void forNowInsert(int pos, QString value);
};

#endif // MAINWINDOW_H

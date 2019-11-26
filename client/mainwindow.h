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
    explicit MainWindow(Socket *sock = nullptr, FileHandler *fileHand = nullptr,QWidget *parent = nullptr);
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
    void fileIsHere();
    void changeViewAfterInsert(QChar l, int pos);
//    void changeViewAfterDelete();

private:
    Ui::MainWindow *ui;
    Socket *socket;
    Dialog *dialog;
    FileHandler *fHandler;
    int letterCounter = 0;

signals:
    void myInsert(int externalIndex, QChar newLetterValue, int clientID);
    void myDelete(int externalIndex);
    void sendNameFile(QString fileNameTmp);
    void newFile(QString filename);
};

#endif // MAINWINDOW_H

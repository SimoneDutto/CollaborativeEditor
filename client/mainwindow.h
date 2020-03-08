#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QColor>
#include "socket.h"
#include "dialog.h"
#include "form.h"
#include "account.h"
#include "uri.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Socket *sock = nullptr, FileHandler *fileHand = nullptr,QWidget *parent = nullptr, QString nome = nullptr);
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
    void changeViewAfterInsert(QChar l, int pos, QTextCharFormat format);
    void changeViewAfterDelete(int externalIndex);
    void changeViewAfterStyle(QString firstLetterID, QString lastLetterID);
    void changeViewAfterCursor(int userID, int position);
    void changeViewAfterConnectedUser(int,int,QColor);
    void ChangeViewAfterDisconnectedUser(int);

    void on_textEdit_cursorPositionChanged();

    void on_actionLog_Out_triggered();

    void on_actionEdit_Profile_triggered();

    void on_actionGet_URI_triggered();

private:
    Ui::MainWindow *ui;
    Socket *socket;
    Dialog *dialog;
    FileHandler *fHandler;
    Form *form;
    Account *account;
    Uri *uri;
    int letterCounter = 0;

signals:
    void myInsert(int externalIndex, QChar newLetterValue, int clientID, QTextCharFormat format);
    void myDelete(int firstExternalIndex, int lastExternalIndex);
    void sendNameFile(QString fileNameTmp);
    void newFile(QString filename);
    void styleChange(QMap<QString, QTextCharFormat>, QString startID, QString lastID, bool boldTriggered, bool italicTriggered, bool underlinedTriggered);
    //void localStyleChange(QString initialID, QString finalID, QTextCharFormat);
    void cursorPositionChange(int position);
};

#endif // MAINWINDOW_H

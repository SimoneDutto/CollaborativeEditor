#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "socket.h"
#include "dialog.h"
#include "form.h"
#include "account.h"
#include "uri.h"
#include "onlineuser.h"
#include "clickablelabel.h"
#include "usersletterswindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Socket *sock = nullptr, FileHandler *fileHand = nullptr,QWidget *parent = nullptr, QString nome = nullptr);
    static bool sorting(QPair<QPair<int,QColor>,int> &, QPair<QPair<int,QColor>,int> &);
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
    void fileIsHere(QMap<int,int>, QMap<int,QColor>);
    void changeViewAfterInsert(QChar l, int pos, QTextCharFormat format);
    void changeViewAfterDelete(int externalIndex);
    void changeViewAfterStyle(QString firstLetterID, QString lastLetterID);
    void addUserConnection(QString username, QColor colorUser);
    void removeUserDisconnect(QString username, int userID);
    void currentFontChanged(QFont font);
    void fontSizeChanged(int size);
    void uploadHistory(QMap<int, QString> mapUsers);


    void on_textEdit_cursorPositionChanged();

    void on_actionLog_Out_triggered();

    void on_actionEdit_Profile_triggered();

    //void on_actionGet_URI_triggered();

    void on_actionExport_as_PDF_triggered();
    //void changeViewAfterCursor(int pos, QColor color);

    void on_counter_clicked();
    void on_write_uri(QString uri);

    void on_actionAlign_to_Left_triggered();

    void on_actionAlign_to_Right_triggered();

    void on_actionAlign_to_Center_triggered();

    void on_actionAlign_to_Justify_triggered();
    void notConnected();

    void on_cursor_triggered(QPair<int,int> idpos, QColor col);

    void changeClientImage(QString path);


    void on_actionhistory_triggered();

private:
    Ui::MainWindow *ui;
    Socket *socket;
    Dialog *dialog;
    FileHandler *fHandler;
    Form *form;
    Account *account;
    int letterCounter = 0;
    QList<QPair<QPair<int,QColor>,int>> id_colore_cursore;


signals:
    void myInsert(int externalIndex, QChar newLetterValue, int clientID, QTextCharFormat format);
    void myDelete(int firstExternalIndex, int lastExternalIndex);
    void sendNameFile(QString fileNameTmp);
    void newFile(QString filename);
    void styleChange(QMap<QString, QTextCharFormat>, QString startID, QString lastID, bool boldTriggered, bool italicTriggered, bool underlinedTriggered, QString font);
    void exportAsPDF();
    void logOut();
    void sendCursorChange(int position);
    //void localStyleChange(QString initialID, QString finalID, QTextCharFormat);
    void setCurrFont(QFont currFont);
    void setCurrFontSize(int sizeFont);
    void sendHist();
    void sendAlignment(Qt::AlignmentFlag alignment, int cursorPosition);
};

#endif // MAINWINDOW_H

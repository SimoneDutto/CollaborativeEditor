#include "pastedtextedit.h"

pastedTextEdit::pastedTextEdit(QWidget *parent) : QTextEdit (parent)
{

}

void pastedTextEdit::insertFromMimeData(const QMimeData *source){

    if(source->hasText()){
        QString text = source->text();
        emit pastedText(text);
    }
}

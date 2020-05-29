#include "pastedtextedit.h"

pastedTextEdit::pastedTextEdit(QWidget *parent) : QTextEdit (parent)
{

}

void pastedTextEdit::insertFromMimeData(const QMimeData *source){

    if(source->hasHtml() && source->hasText()){
        QString html = source->html();
        QString text = source->text();
        emit pastedText(html, text);
    }
}

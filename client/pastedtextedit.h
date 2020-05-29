#ifndef PASTEDTEXTEDIT_H
#define PASTEDTEXTEDIT_H

#include <QWidget>
#include <QTextEdit>
#include <QDebug>
#include <QMimeData>
#include "mainwindow.h"

class pastedTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit pastedTextEdit(QWidget *parent = nullptr);
    void insertFromMimeData(const QMimeData *source);

signals:
    void pastedText(QString html, QString text);
};

#endif // PASTEDTEXTEDIT_H

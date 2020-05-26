QT       += core gui
QT += network widgets
QT += quickcontrols2
QT += printsupport
TARGET = C++ollaborativeEditor

requires(qtConfig(combobox))
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    account.cpp \
    clickablelabel.cpp \
    dialog.cpp \
    error.cpp \
    newopen.cpp \
    filehandler.cpp \
    form.cpp \
    letter.cpp \
    main.cpp \
    login.cpp \
    mainwindow.cpp \
    onlineuser.cpp \
    pastedtextedit.cpp \
    serverdisc.cpp \
    signup.cpp \
    socket.cpp \
    uri.cpp \
    usersletterswindow.cpp \
    welcome.cpp

HEADERS += \
    account.h \
    clickablelabel.h \
    dialog.h \
    error.h \
    newopen.h \
    filehandler.h \
    form.h \
    letter.h \
    login.h \
    mainwindow.h \
    onlineuser.h \
    pastedtextedit.h \
    serverdisc.h \
    signup.h \
    socket.h \
    uri.h \
    usersletterswindow.h \
    welcome.h

FORMS += \
    account.ui \
    dialog.ui \
    error.ui \
    newopen.ui \
    form.ui \
    login.ui \
    mainwindow.ui \
    onlineuser.ui \
    serverdisc.ui \
    signup.ui \
    socket.ui \
    uri.ui \
    usersletterswindow.ui \
    welcome.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

DISTFILES +=

ICON = icone/logo_transparent copy.png

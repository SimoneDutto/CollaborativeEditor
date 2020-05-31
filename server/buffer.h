#ifndef BUFFER_H
#define BUFFER_H

#include <QByteArray>

class Buffer
{
public:
    Buffer();
    QByteArray data;
    quint64 dim = 0;
};

#endif // BUFFER_H

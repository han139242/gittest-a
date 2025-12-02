#ifndef HAFFMAN_H
#define HAFFMAN_H

#include <QMainWindow>
#include <QObject>
#include <QQuickItem>
#include <QSharedDataPointer>
#include <QWidget>

class haffmanData;

class haffman
{
public:
    haffman();
    haffman(const haffman &);
    haffman(haffman &&);
    haffman &operator=(const haffman &);
    haffman &operator=(haffman &&);
    ~haffman();

private:
    QSharedDataPointer<haffmanData> data;
};

#endif // HAFFMAN_H

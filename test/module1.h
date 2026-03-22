#ifndef MODULE1_H
#define MODULE1_H

#include "widgets/tablewidget.h"

#include <QEventLoop>
#include <QTimer>
#include <QDebug>

class Module1 : public TableWidget
{
    Q_OBJECT
public:
    explicit Module1(QWidget *parent = nullptr);

    bool test1();
    bool test2();
    bool test3();
};

#endif // MODULE1_H

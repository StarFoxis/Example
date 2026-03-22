#ifndef MODULE2_H
#define MODULE2_H

#include "widgets/tablewidget.h"

#include <QEventLoop>
#include <QTimer>
#include <QDebug>

class Module2 : public TableWidget
{
    Q_OBJECT
public:
    explicit Module2(QWidget *parent = nullptr);

    bool test1();
};

#endif // MODULE2_H

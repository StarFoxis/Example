#ifndef TESTS_H
#define TESTS_H

#include "uart.h"
#include "widgets/tablewidget.h"

#include <QWidget>
#include <QSettings>
#include <QTimer>
#include <QList>
#include <QPair>

QT_BEGIN_NAMESPACE
namespace Ui {
class Tests;
}
QT_END_NAMESPACE

class Tests : public QWidget
{
    Q_OBJECT

public:
    explicit Tests(QWidget *parent = nullptr);
    ~Tests();

    void saveSettings(QSettings* settings);
    void loadSettings(QSettings* settings);
    void setUART(UART* uart);

private:
    Ui::Tests* ui;
    UART* mUART = nullptr;

    bool mTestIsRunning = false;
    QList<QPair<int, QString>> mActiveTests;
    int mCurrentTestIndex = -1;

    void initWidgets();
    void initConnects();

    void startingTests();
    void stopTests();
    void finishTests();
    void executeTests();
    TableWidget* getCurrentTableWidget();

private slots:
    void onPushButtonStartTests();

signals:
    void signalNewMessage(const QString& message, MessageType messageType = Info);
};

#endif // TESTS_H

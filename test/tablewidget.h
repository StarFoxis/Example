#ifndef TABLEWIDGET_H
#define TABLEWIDGET_H

#include "uart.h"

#include <QTableWidget>
#include <QCheckBox>
#include <functional>

class TableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit TableWidget(QWidget *parent = nullptr);
    ~TableWidget() = default;

    void setUART(UART* uart);
    void insertTest(int row, const QString &testName, std::function<bool()> testFunction);
    bool runTest(int index);
    bool isTestEnabled(int row) const;

    QString getName(int row);

protected:
    UART* mUART = nullptr;
    QStringList mTestNames;
    QList<QCheckBox*> mCheckBoxes;
    QList<std::function<bool()>> mTestFunctions;

private:
    void createCheckBox(int row);

private slots:
    void onItemClicked(QTableWidgetItem* item);

signals:
    void signalNewMessage(const QString& message, MessageType messageType = Info);
};

#endif // TABLEWIDGET_H

#ifndef TERMINAL_H
#define TERMINAL_H

#include "uart.h"
#include "tests.h"

#include <QFrame>
#include <QDateTime>
#include <QMainWindow>
#include <QCloseEvent>
#include <QDockWidget>
#include <QTimer>
#include <QList>
#include <QSettings>
#include <QSerialPort>
#include <QTabWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class Terminal;
}
QT_END_NAMESPACE

class Terminal : public QMainWindow
{
    Q_OBJECT

public:
    Terminal(QWidget *parent = nullptr);
    ~Terminal();

private:
    Ui::Terminal *ui;
    UART* mUART;
    Tests* mTests;

    QDockWidget* mCommandsDock;      // Dock для команд
    QDockWidget* mTestsDock;         // Dock для тестов

    QString filenameSettings;
    QString styleLight;
    QString styleDark;
    QString currentStyle = "light";

    struct MessageHistory {
        QString text;
        MessageType type;
        QTime time;
    };

    QList<MessageHistory> messageHistory;

    struct Packet {
        int index = -1;
        QString name;
        QString command;
        int repeat = 1000;
        bool state = false;
        QTimer* timer = nullptr;
        QFrame* frame = nullptr;

        Packet() = default;
        Packet(int idx, const QString& n, const QString& cmd, int rpt, bool st)
            : index(idx), name(n), command(cmd), repeat(rpt), state(st) {}
    };

    int currentIndexPacket = 0;
    QList<Packet> bufferPackets;

    void initWidgets();
    void initConnects();
    void initComboBoxUART();
    void loadSettings(const QString& filename);
    void saveSettings(const QString& filename);
    void openUART();
    void closeUART();
    void addPacket(const Packet& packet);
    void applyStyleSheet();
    void updateMessagesColors();
    void setupDockWidgets();

public slots:
    void displayMessage(const QString& message, MessageType messageType = Info);

private slots:
    void onPushButtonUpdatePortNamesClicked();
    void onPushButtonControllUARTClicked();
    void onPushButtonCommandUARTClicked();
    void onPushButtonCommandListClicked();
    void onToolButtonStyleClicked();
    void onPushButtonOpenTests();
    void onPushButtonMacrosClicked();

    void onToolButtonIndexClick();
    void onPushButtonExecuteClicked();
    void onSpinBoxRepeatValueChanged(int value);
    void onCheckBoxStateToggled(bool state);
    void onTimerExecuteTimeout();

    void onCommandsDockVisibilityChanged(bool visible);
    void onTestsDockVisibilityChanged(bool visible);

protected:
    void closeEvent(QCloseEvent* event) override;
};

#endif // TERMINAL_H

#ifndef UART_H
#define UART_H

#include "global.h"

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>

#define START 0xCD

#define MAXBYTE 100 // Максимальный размер пакета

class UART : public QObject
{
    Q_OBJECT
public:
    explicit UART(QObject *parent = nullptr);

    bool open(const QString& portName,
              QSerialPort::BaudRate baudRate,
              QSerialPort::DataBits dataBits,
              QSerialPort::FlowControl flowControl,
              QSerialPort::Parity parity,
              QSerialPort::StopBits stopBits);
    void close();
    bool isOpen();
    bool writeData(const QByteArray& data);
    bool waitForCondition(int timeoutMs);
    QByteArray buildPacket(QByteArray data);
    QStringList getPortNames();

    QByteArray buffer;

private:
    enum WaitForState {
        waitStart,
        waitSize,
        waitData,
        waitCRC
    };

    QSerialPort* mPort;    
    WaitForState waitForState;
    int expectedSize;

    bool processRead(quint8 byte);
    void resetState();
    quint8 getCRC(const QByteArray& data);
    bool validateCRC(quint8 crc);
    void processCompletePacket();

private slots:
    void onReadyRead();

signals:
    void signalNewMessage(const QString& message, MessageType messageType = Info);
    void dataReceived();
};

#endif // UART_H

#include "uart.h"

#include <QDebug>
#include <QTimer>
#include <QEventLoop>

UART::UART(QObject *parent)
    : QObject{parent}
{
    mPort = new QSerialPort(this);
    connect(mPort, &QSerialPort::readyRead, this, &UART::onReadyRead);
}

/**
 * @brief Открывает последовательный порт с заданными параметрами
 *
 * Данная функция инициализирует и открывает последовательный порт
 * с указанными параметрами соединения. В случае успешного открытия
 * порта, он становится доступным для чтения и записи.
 *
 * @param portName Имя последовательного порта
 * @param baudRate Скорость передачи данных в бодах
 * @param dataBits Количество битов данных в кадре
 * @param flowControl Метод управления потоком
 * @param parity Тип контроля четности
 * @param stopBits Количество стоповых битов
 *
 * @return true если порт успешно открыт, false в случае ошибки
 *
 */
bool UART::open(const QString &portName,
                QSerialPort::BaudRate baudRate,
                QSerialPort::DataBits dataBits,
                QSerialPort::FlowControl flowControl,
                QSerialPort::Parity parity,
                QSerialPort::StopBits stopBits)
{
    if (mPort->isOpen()) {
        mPort->close();
    }

    mPort->setPortName(portName);
    if (!mPort->setBaudRate(baudRate)) {
        emit signalNewMessage("Не удалось установить скорость порта.", Error);
        return false;
    }
    if (!mPort->setDataBits(dataBits)) {
        emit signalNewMessage("Не удалось установить биты данных.", Error);
        return false;
    }
    if (!mPort->setFlowControl(flowControl)) {
        emit signalNewMessage("Не удалось установить управление потоком.", Error);
        return false;
    }
    if (!mPort->setParity(parity)) {
        emit signalNewMessage("Не удалось установить четность порта.", Error);
        return false;
    }
    if (!mPort->setStopBits(stopBits)) {
        emit signalNewMessage("Не удалось установить стоповые биты.", Error);
        return false;
    }
    if (mPort->open(QSerialPort::ReadWrite)) {
        emit signalNewMessage("Последовательный порт с заданными параметрами успешно подключен.");
        return true;
    }
    else {
        emit signalNewMessage("Не удалось открыть последовательный порт с заданными параметрами.", Error);
        return false;
    }
}

/**
 * @brief Закрывает последовательный порт
 *
 * Данная функция закрывает последовательный порт.
 *
 * @param нет
 *
 * @return нет
 *
 */
void UART::close()
{
    mPort->close();
    emit signalNewMessage("Последовательный порт успешно закрыт.");
}

/**
 * @brief Возвращает текущее состояние порта
 *
 * Данная функция возвращает текущее состояние порта.
 *
 * @param нет
 *
 * @return возвращает текущее состояние порта (true - открыт; false - закрыт)
 *
 */
bool UART::isOpen()
{
    return mPort->isOpen();
}

bool UART::writeData(const QByteArray &data)
{
    if (!mPort->isOpen()) {
        emit signalNewMessage("Проверьте подключение к последовательному порту.", Error);
        return false;
    }

    quint64 size = mPort->write(data);

    if (size > 0) {
        emit signalNewMessage(QString("Записано %1 байт из %2.").arg(size).arg(data.size()));
        emit signalNewMessage(QString("Данные: %1;").arg(data.toHex(' ')));
    }
    else {
        emit signalNewMessage("Не удалось отправить данные.", Error);
    }

    return size;
}

QByteArray UART::buildPacket(QByteArray data)
{
    QByteArray packet;
    packet.append(START);
    packet.append(quint8(data.size()));
    packet.append(data);
    packet.append(getCRC(data));
    return packet;
}

QStringList UART::getPortNames()
{
    QStringList portNames;
    foreach (auto portInfo, QSerialPortInfo::availablePorts()) {
        portNames << portInfo.portName();
    }
    portNames.sort();
    return portNames;
}

bool UART::processRead(quint8 byte)
{
    switch (waitForState) {
    case waitStart:
        if (byte == START) {
            buffer.clear();
            waitForState = waitSize;
        }
        break;
    case waitSize:
        // buffer.append(byte);
        expectedSize = byte;
        waitForState = waitData;
        break;
    case waitData:
        buffer.append(byte);
        if (buffer.size() == expectedSize) {
            waitForState = waitCRC;
        }
        else if (buffer.size() > MAXBYTE) {
            emit signalNewMessage("Данные превысили максимальный размер.", Error);
            return false;
        }
        break;
    case waitCRC:
        if (!validateCRC(byte)) {
            emit signalNewMessage("Не верная контрольная сумма.", Error);
            return false;
        }
        else {
            waitForState = waitStart;
            processCompletePacket();
            return true;
        }
        break;
    }

    return true;
}

quint8 UART::getCRC(const QByteArray &data)
{
    quint8 crc = 0;
    foreach (quint8 byte, data) {
        crc ^= byte;
    }
    return crc;
}

bool UART::validateCRC(quint8 crc)
{
    quint8 c = getCRC(buffer);
    return crc == c;
}

void UART::processCompletePacket()
{
    emit signalNewMessage(QString("Принято: %1 байт.").arg(buffer.size()));
    emit signalNewMessage(QString("Данные: %1;").arg(buffer.toHex(' ')));
    emit dataReceived();
}

void UART::resetState()
{
    waitForState = waitStart;
    expectedSize = 0;
    buffer.clear();
}

void UART::onReadyRead()
{
    if (mPort->bytesAvailable()) {
        QByteArray incomingBuffer = mPort->readAll();

        foreach (quint8 byte, incomingBuffer) {
            if (!processRead(byte)) {
                resetState();
                // TODO:
                qDebug() << "Ошибка пакета";
            }
        }
    }
}

bool UART::waitForCondition(int timeoutMs)
{
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    bool timeout = false;

    connect(&timer, &QTimer::timeout, &loop, [&loop, &timeout]() {
        timeout = true;
        loop.quit();
    });

    connect(this, &UART::dataReceived, &loop, [&loop](){
        loop.quit();
    });

    timer.start(timeoutMs);
    loop.exec();

    return !timeout;
}

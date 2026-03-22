#include "module1.h"

Module1::Module1(QWidget *parent)
    : TableWidget{parent}
{
    insertTest(0, "Тест связи", [this]() { return test1(); });
    insertTest(1, "Тест памяти", [this]() { return test2(); });
    insertTest(2, "Тест процессора", [this]() { return test3(); });
}

bool Module1::test1()
{
    // Проверяем, открыт ли UART
    if (!mUART->isOpen()) {
        emit signalNewMessage("UART не открыт", Error);
        return false;
    }

    // Отправляем команду
    QByteArray command = QByteArray::fromHex("AA 00 01 BB 00");
    if (!mUART->writeData(mUART->buildPacket(command))) {
        emit signalNewMessage(QString("Команда %1 не была отправлена").arg(command.toHex(' ')), Error);
        return false;
    }

    // Ждём ответ
    if (!mUART->waitForCondition(3000)) {
        emit signalNewMessage("Таймаут ожидания ответа", Error);
        return false;
    }

    // TODO: Обработка пакета

    return true;
}

bool Module1::test2()
{
    // Проверяем, открыт ли UART
    if (!mUART->isOpen()) {
        emit signalNewMessage("UART не открыт", Error);
        return false;
    }

    // Отправляем команду
    QByteArray command = QByteArray::fromHex("AA 00 01 BB 01");
    if (!mUART->writeData(mUART->buildPacket(command))) {
        emit signalNewMessage(QString("Команда %1 не была отправлена").arg(command.toHex(' ')), Error);
        return false;
    }

    // Ждём ответ
    if (!mUART->waitForCondition(3000)) {
        emit signalNewMessage("Таймаут ожидания ответа", Error);
        return false;
    }

    // TODO: Обработка пакета

    return true;
}

bool Module1::test3()
{
    // Проверяем, открыт ли UART
    if (!mUART->isOpen()) {
        emit signalNewMessage("Ошибка. UART не открыт", Error);
        return false;
    }

    // Отправляем команду
    QByteArray command = QByteArray::fromHex("AA 00 01 BB 00");
    if (!mUART->writeData(mUART->buildPacket(command))) {
        emit signalNewMessage(QString("Команда %1 не была отправлена").arg(command.toHex(' ')), Error);
        return false;
    }

    // Ждём ответ
    if (!mUART->waitForCondition(3000)) {
        emit signalNewMessage("Таймаут ожидания ответа", Error);
        return false;
    }

    // TODO: Обработка пакета

    return true;
}


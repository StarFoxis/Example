#include "module2.h"

Module2::Module2(QWidget *parent)
    : TableWidget{parent}
{
    insertTest(0, "Тест компонентов", [this]() { return test1(); });
}

bool Module2::test1()
{
    // Проверяем, открыт ли UART
    if (!mUART->isOpen()) {
        emit signalNewMessage("UART не открыт", Error);
        return false;
    }

    // Отправляем команду
    QByteArray command = QByteArray::fromHex("AA 00 02 BB 00");
    if (!mUART->writeData(mUART->buildPacket(command))) {
        emit signalNewMessage(QString("Команда %1 не была отправлена").arg(command.toHex(' ')), Error);
        return false;
    }

    // Ждём ответ
    if (!mUART->waitForCondition(2000)) {
        emit signalNewMessage("Таймаут ожидания ответа", Error);
        return false;
    }

    // TODO: Обработка пакета

    return true;
}

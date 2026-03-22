#include "terminal.h"
#include "ui_terminal.h"

#include <QFile>
#include <QTimer>
#include <QSpinBox>
#include <QSettings>
#include <QCheckBox>
#include <QTextBlock>
#include <QToolButton>
#include <QRegularExpressionValidator>
#include <QHBoxLayout>
#include <QMessageBox>

Terminal::Terminal(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Terminal)
{
    ui->setupUi(this);

    filenameSettings = qApp->applicationDirPath() + "/Options/settings.ini";

    initWidgets();
    initConnects();
    loadSettings(filenameSettings);
}

Terminal::~Terminal()
{
    delete ui;
}

void Terminal::setupDockWidgets()
{
    // Создаем док-панели
    mCommandsDock = new QDockWidget(this);
    mCommandsDock->setObjectName("CommandsDock");
    mTestsDock = new QDockWidget(this);
    mTestsDock->setObjectName("TestsDock");

    // Настраиваем свойства док-панелей
    mCommandsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mTestsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    // Разрешаем закрытие (скрытие) панелей
    mCommandsDock->setFeatures(QDockWidget::DockWidgetClosable |
                               QDockWidget::DockWidgetMovable |
                               QDockWidget::DockWidgetFloatable);
    mTestsDock->setFeatures(QDockWidget::DockWidgetClosable |
                            QDockWidget::DockWidgetMovable |
                            QDockWidget::DockWidgetFloatable);

    // Извлекаем виджеты из UI и добавляем их в док-панели
    QLayout* layout = ui->centralwidget->layout();
    if (layout) {
        layout->removeWidget(ui->frameWritterCommand);
    }

    mCommandsDock->setWidget(ui->frameWritterCommand);
    mTestsDock->setWidget(mTests);

    // Добавляем док-панели в главное окно
    addDockWidget(Qt::LeftDockWidgetArea, mCommandsDock);
    addDockWidget(Qt::LeftDockWidgetArea, mTestsDock);

    // Изначально скрываем панель тестов
    mCommandsDock->hide();
    mTestsDock->show();

    // Подключаем сигналы изменения видимости
    connect(mCommandsDock, &QDockWidget::visibilityChanged,
            this, &Terminal::onCommandsDockVisibilityChanged);
    connect(mTestsDock, &QDockWidget::visibilityChanged,
            this, &Terminal::onTestsDockVisibilityChanged);
}

void Terminal::initWidgets()
{
    mUART = new UART(this);
    mTests = new Tests(this);
    mTests->setUART(mUART);

    QRegularExpressionValidator* validatorHex = new QRegularExpressionValidator(
        QRegularExpression("^(?i)([0-9A-F]{2})(\\s[0-9A-F]{2})*$"), this);
    ui->lineEditCommandPacket->setValidator(validatorHex);

    initComboBoxUART();
    setupDockWidgets();

    QFile styleLightFile(":/styles/light.qss");
    if (styleLightFile.open(QFile::ReadOnly)) {
        styleLight = QLatin1String(styleLightFile.readAll());
        styleLightFile.close();
    }
    QFile styleDarkFile(":/styles/dark.qss");
    if (styleDarkFile.open(QFile::ReadOnly)) {
        styleDark = QLatin1String(styleDarkFile.readAll());
        styleDarkFile.close();
    }
}

void Terminal::initComboBoxUART()
{
    ui->comboBoxFlowControl->clear();
    ui->comboBoxPortName->clear();
    ui->comboBoxBaudRate->clear();
    ui->comboBoxDataBits->clear();
    ui->comboBoxStopBits->clear();
    ui->comboBoxParity->clear();

    // Название порта
    QStringList portNames = mUART->getPortNames();
    if (portNames.isEmpty()) {
        ui->comboBoxPortName->addItem("Нет доступных");
    }
    else {
        ui->comboBoxPortName->addItems(portNames);
    }

    // Скорость порта
    ui->comboBoxBaudRate->addItem("1200", QSerialPort::Baud1200);
    ui->comboBoxBaudRate->addItem("2400", QSerialPort::Baud2400);
    ui->comboBoxBaudRate->addItem("4800", QSerialPort::Baud4800);
    ui->comboBoxBaudRate->addItem("9600", QSerialPort::Baud9600);
    ui->comboBoxBaudRate->addItem("19200", QSerialPort::Baud19200);
    ui->comboBoxBaudRate->addItem("38400", QSerialPort::Baud38400);
    ui->comboBoxBaudRate->addItem("57600", QSerialPort::Baud57600);
    ui->comboBoxBaudRate->addItem("115200", QSerialPort::Baud115200);

    // Биты данных
    ui->comboBoxDataBits->addItem("5 бит", QSerialPort::Data5);
    ui->comboBoxDataBits->addItem("6 бит", QSerialPort::Data6);
    ui->comboBoxDataBits->addItem("7 бит", QSerialPort::Data7);
    ui->comboBoxDataBits->addItem("8 бит", QSerialPort::Data8);

    // Четность
    ui->comboBoxParity->addItem("Без контроля", QSerialPort::NoParity);
    ui->comboBoxParity->addItem("Нечетный контроль", QSerialPort::OddParity);
    ui->comboBoxParity->addItem("Четный контроль", QSerialPort::EvenParity);
    ui->comboBoxParity->addItem("Маркерный контроль", QSerialPort::MarkParity);
    ui->comboBoxParity->addItem("Пробельный контроль", QSerialPort::SpaceParity);

    // Стоповые биты
    ui->comboBoxStopBits->addItem("1 бит", QSerialPort::OneStop);
    ui->comboBoxStopBits->addItem("2 бит", QSerialPort::TwoStop);
    ui->comboBoxStopBits->addItem("1.5 бит", QSerialPort::OneAndHalfStop);

    // Контроль потока
    ui->comboBoxFlowControl->addItem("Без управления", QSerialPort::NoFlowControl);
    ui->comboBoxFlowControl->addItem("Аппаратное управление", QSerialPort::HardwareControl);
    ui->comboBoxFlowControl->addItem("Программное управление", QSerialPort::SoftwareControl);
}

void Terminal::initConnects()
{
    // Сигналы/слоты UI
    connect(ui->pushButtonUpdatePortNames, &QPushButton::clicked,
            this, &Terminal::onPushButtonUpdatePortNamesClicked);
    connect(ui->pushButtonControlUART, &QPushButton::clicked,
            this, &Terminal::onPushButtonControllUARTClicked);
    connect(ui->pushButtonPushCommandUART, &QPushButton::clicked,
            this, &Terminal::onPushButtonCommandUARTClicked);
    connect(ui->pushButtonPushCommandList, &QPushButton::clicked,
            this, &Terminal::onPushButtonCommandListClicked);
    connect(ui->toolButtonStyle, &QToolButton::clicked,
            this, &Terminal::onToolButtonStyleClicked);
    connect(ui->pushButtonOpenMacros, &QToolButton::clicked,
            this, &Terminal::onPushButtonMacrosClicked);
    connect(ui->pushButtonOpenTests, &QPushButton::clicked,
            this, &Terminal::onPushButtonOpenTests);

    // Сигналы/слоты приёма сообщений
    connect(mUART, &UART::signalNewMessage, this, &Terminal::displayMessage);
    connect(mTests, &Tests::signalNewMessage, this, &Terminal::displayMessage);

    connect(ui->textBrowserMessage, &TextBrowser::signalClearHistory, this, [=](){
        messageHistory.clear();
    });
}

void Terminal::loadSettings(const QString &filename)
{
    QSettings settings(filename, QSettings::IniFormat);

    settings.beginGroup("window");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());

    ui->textBrowserMessage->setZoomStep(settings.value("terminal_zoom_step", ui->textBrowserMessage->getZoomStep()).toInt());

    currentStyle = settings.value("style", "light").toString();
    ui->toolButtonStyle->setText(currentStyle == "light" ? "Светлая" : "Тёмная");
    applyStyleSheet();
    settings.endGroup();

    settings.beginGroup("uart");
    auto loadCombo = [&](QComboBox* combo, const QString& key, const QString& defaultValue) {
        combo->setCurrentText(settings.value(key, defaultValue).toString());
    };

    loadCombo(ui->comboBoxPortName, "portName", "Нет доступных");
    loadCombo(ui->comboBoxBaudRate, "baudRate", "115200");
    loadCombo(ui->comboBoxDataBits, "dataBits", "8 бит");
    loadCombo(ui->comboBoxStopBits, "stopBits", "1 бит");
    loadCombo(ui->comboBoxFlowControl, "flowControl", "Без управления");
    loadCombo(ui->comboBoxParity, "parity", "Без контроля");

    // Загрузка пакетов
    QByteArray packetsData = settings.value("packets").toByteArray();
    if (!packetsData.isEmpty()) {
        QDataStream stream(&packetsData, QDataStream::ReadOnly);
        int size;
        stream >> size;

        for (int i = 0; i < size; ++i) {
            Packet packet;
            stream >> packet.index >> packet.name >> packet.command
                >> packet.repeat >> packet.state;
            addPacket(packet);
        }
        currentIndexPacket = size;
    }
    settings.endGroup();

    mTests->loadSettings(&settings);
}

void Terminal::saveSettings(const QString &filename)
{
    QSettings settings(filename, QSettings::IniFormat);

    settings.beginGroup("window");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());

    settings.setValue("style", currentStyle);
    settings.setValue("terminal_zoom_step", ui->textBrowserMessage->getZoomStep());
    settings.endGroup();

    settings.beginGroup("uart");
    if (ui->comboBoxPortName->count() > 0 &&
        ui->comboBoxPortName->currentText() != "Нет доступных") {
        settings.setValue("portName", ui->comboBoxPortName->currentText());
    }
    settings.setValue("baudRate", ui->comboBoxBaudRate->currentText());
    settings.setValue("dataBits", ui->comboBoxDataBits->currentText());
    settings.setValue("stopBits", ui->comboBoxStopBits->currentText());
    settings.setValue("flowControl", ui->comboBoxFlowControl->currentText());
    settings.setValue("parity", ui->comboBoxParity->currentText());

    QByteArray packets;
    QDataStream stream(&packets, QDataStream::WriteOnly);
    int sizePackets = bufferPackets.size();
    stream << sizePackets;
    foreach (Packet packet, bufferPackets) {
        stream << packet.index;
        stream << packet.name;
        stream << packet.command;
        stream << packet.repeat;
        stream << packet.state;
    }
    settings.setValue("packets", packets);
    settings.endGroup();

    mTests->saveSettings(&settings);
    settings.sync();
}

void Terminal::openUART()
{
    if (mUART->isOpen()) {
        mUART->close();
    }

    QString portName = ui->comboBoxPortName->currentText();
    QSerialPort::BaudRate baudRate = ui->comboBoxBaudRate->currentData().value<QSerialPort::BaudRate>();
    QSerialPort::DataBits dataBits = ui->comboBoxDataBits->currentData().value<QSerialPort::DataBits>();
    QSerialPort::StopBits stopBits = ui->comboBoxStopBits->currentData().value<QSerialPort::StopBits>();
    QSerialPort::FlowControl flowControl = ui->comboBoxFlowControl->currentData().value<QSerialPort::FlowControl>();
    QSerialPort::Parity parity = ui->comboBoxParity->currentData().value<QSerialPort::Parity>();

    if (mUART->open(portName, baudRate, dataBits, flowControl, parity, stopBits)) {
        ui->pushButtonControlUART->setText("Отключить");
        ui->comboBoxPortName->setEnabled(false);
        ui->comboBoxBaudRate->setEnabled(false);
        ui->comboBoxDataBits->setEnabled(false);
        ui->comboBoxStopBits->setEnabled(false);
        ui->comboBoxFlowControl->setEnabled(false);
        ui->comboBoxParity->setEnabled(false);
    }
}

void Terminal::closeUART()
{
    mUART->close();

    ui->pushButtonControlUART->setText("Подключить");
    ui->comboBoxPortName->setEnabled(true);
    ui->comboBoxBaudRate->setEnabled(true);
    ui->comboBoxDataBits->setEnabled(true);
    ui->comboBoxStopBits->setEnabled(true);
    ui->comboBoxFlowControl->setEnabled(true);
    ui->comboBoxParity->setEnabled(true);
}

void Terminal::addPacket(const Packet& packet)
{
    auto* frame = new QFrame();

    auto* toolButtonIndex = new QToolButton(frame);
    toolButtonIndex->setText(QString::number(packet.index + 1));
    toolButtonIndex->setProperty("index", packet.index);
    connect(toolButtonIndex, &QToolButton::clicked, this, &Terminal::onToolButtonIndexClick);

    auto* lineEditPacketName = new QLineEdit(packet.name, frame);
    lineEditPacketName->setReadOnly(true);

    auto* lineEditPacketInfo = new QLineEdit(packet.command, frame);
    lineEditPacketInfo->setReadOnly(true);

    auto* pushButtonExecute = new QPushButton("Выполнить", frame);
    pushButtonExecute->setProperty("index", packet.index);
    connect(pushButtonExecute, &QPushButton::clicked, this, &Terminal::onPushButtonExecuteClicked);

    auto* spinBoxRepeat = new QSpinBox(frame);
    spinBoxRepeat->setRange(1, 5000);
    spinBoxRepeat->setValue(packet.repeat);
    spinBoxRepeat->setProperty("index", packet.index);
    spinBoxRepeat->setMaximumWidth(80);
    connect(spinBoxRepeat, QOverload<int>::of(&QSpinBox::valueChanged), this, &Terminal::onSpinBoxRepeatValueChanged);

    auto* checkBoxState = new QCheckBox(frame);
    checkBoxState->setChecked(packet.state);
    checkBoxState->setProperty("index", packet.index);
    connect(checkBoxState, &QCheckBox::toggled, this, &Terminal::onCheckBoxStateToggled);

    auto* timerExecute = new QTimer(frame);
    timerExecute->setProperty("index", packet.index);
    timerExecute->setInterval(packet.repeat);
    connect(timerExecute, &QTimer::timeout, this, &Terminal::onTimerExecuteTimeout);

    auto* hbox = new QHBoxLayout(frame);
    hbox->addWidget(toolButtonIndex);
    hbox->addWidget(lineEditPacketInfo, 1);
    hbox->addWidget(lineEditPacketName, 1);
    hbox->addWidget(pushButtonExecute);
    hbox->addWidget(spinBoxRepeat);
    hbox->addWidget(checkBoxState);
    hbox->setContentsMargins(4, 4, 4, 4);
    hbox->setSpacing(3);

    frame->setLayout(hbox);

    Packet newPacket = packet;
    newPacket.timer = timerExecute;
    newPacket.frame = frame;
    bufferPackets.append(newPacket);

    ui->scrollAreaWidgetContentsCommandUART->layout()->addWidget(frame);
}

void Terminal::applyStyleSheet()
{
    if (currentStyle == "light") {
        setStyleSheet(styleLight);
    }
    else if (currentStyle == "dark") {
        setStyleSheet(styleDark);
    }
}

void Terminal::updateMessagesColors()
{
    ui->textBrowserMessage->clear();

    for (const auto &msg : messageHistory) {
        QString color;
        QString timeColor;

        if (currentStyle == "dark") {
            timeColor = "#969696";
            switch (msg.type) {
            case Error:   color = "#ff6464"; break;
            case Info:    color = "#c8c8c8"; break;
            case Success: color = "#64ff64"; break;
            case Warning: color = "#ffaa44"; break;
            }
        } else {
            timeColor = "#646464";
            switch (msg.type) {
            case Error:   color = "#c80000"; break;
            case Info:    color = "#000000"; break;
            case Success: color = "#009600"; break;
            case Warning: color = "#cc8800"; break;
            }
        }

        QString timeStr = msg.time.toString("hh:mm:ss");
        QString html = QString(
                           "<span style='color: %1;'>[%2]</span> "
                           "<span style='color: %3;'>%4</span>"
                           ).arg(timeColor, timeStr, color, msg.text.toHtmlEscaped());

        ui->textBrowserMessage->append(html);
    }

    ui->textBrowserMessage->moveCursor(QTextCursor::End);
    ui->textBrowserMessage->ensureCursorVisible();
}

void Terminal::displayMessage(const QString &message, MessageType messageType)
{
    MessageHistory history;
    history.text = message;
    history.type = messageType;
    history.time = QTime::currentTime();
    messageHistory.append(history);

    QString color;
    QString timeColor;

    if (currentStyle == "dark") {
        timeColor = "#969696";
        switch (messageType) {
        case Error:   color = "#ff6464"; break;
        case Info:    color = "#c8c8c8"; break;
        case Success: color = "#64ff64"; break;
        case Warning: color = "#ffaa44"; break;
        }
    } else {
        timeColor = "#646464";
        switch (messageType) {
        case Error:   color = "#c80000"; break;
        case Info:    color = "#000000"; break;
        case Success: color = "#009600"; break;
        case Warning: color = "#cc8800"; break;
        }
    }

    QString timeStr = history.time.toString("hh:mm:ss");
    QString html = QString(
                       "<span style='color: %1;'>[%2]</span> "
                       "<span style='color: %3;'>%4</span>"
                       ).arg(timeColor, timeStr, color, message.toHtmlEscaped());

    ui->textBrowserMessage->append(html);
    ui->textBrowserMessage->moveCursor(QTextCursor::End);
}

void Terminal::onPushButtonUpdatePortNamesClicked()
{
    QStringList portNames = mUART->getPortNames();
    ui->comboBoxPortName->clear();
    if (portNames.isEmpty()) {
        ui->comboBoxPortName->addItem("Нет доступных");
    }
    else {
        ui->comboBoxPortName->addItems(portNames);
    }

    if (ui->pushButtonControlUART->text() == "Отключить") {
        closeUART();
    }

    displayMessage("Список последовательных портов обновлен");
}

void Terminal::onPushButtonControllUARTClicked()
{
    if (ui->pushButtonControlUART->text() == "Подключить") {
        openUART();
    }
    else {
        closeUART();
    }
}

void Terminal::onPushButtonCommandUARTClicked()
{
    QString packetInfo = ui->lineEditCommandPacket->text();

    if (!mUART->isOpen()) {
        displayMessage("Проверьте подключение к последовательному порту.", Error);
        return;
    }

    if (packetInfo.isEmpty()) {
        displayMessage("Невозможно выполнить команду меньше 1 байт", Error);
        return;
    }

    QString packetString = packetInfo.split(" ").join("");
    QByteArray command = QByteArray::fromHex(packetString.toLatin1());

    if (mUART->writeData(mUART->buildPacket(command))) {
        displayMessage("Команда отправлена: " + packetInfo, Success);
    } else {
        displayMessage("Ошибка отправки команды", Error);
    }
}

void Terminal::onPushButtonCommandListClicked()
{
    QString packetName = ui->lineEditCommandName->text();
    QString packetInfo = ui->lineEditCommandPacket->text();

    if (packetName.isEmpty()) {
        displayMessage("Невозможно добавить команду без названия.", Error);
        return;
    }
    if (packetInfo.isEmpty()) {
        displayMessage("Невозможно добавить команду меньше 1 байт", Error);
        return;
    }

    Packet packet;
    packet.index = currentIndexPacket++;
    packet.name = packetName;
    packet.command = packetInfo;
    packet.repeat = 1000;
    packet.state = false;
    addPacket(packet);

    ui->lineEditCommandName->clear();
    ui->lineEditCommandPacket->clear();
}

void Terminal::onToolButtonStyleClicked()
{
    if (ui->toolButtonStyle->text() == "Светлая") {
        ui->toolButtonStyle->setText("Тёмная");
        currentStyle = "dark";
    }
    else if (ui->toolButtonStyle->text() == "Тёмная") {
        ui->toolButtonStyle->setText("Светлая");
        currentStyle = "light";
    }

    applyStyleSheet();
    updateMessagesColors();
}

void Terminal::onPushButtonMacrosClicked()
{
    if (!mCommandsDock->isVisible()) {
        mCommandsDock->show();
        mCommandsDock->raise();
    } else {
        mCommandsDock->raise();
    }
}

void Terminal::onPushButtonOpenTests()
{
    if (!mTestsDock->isVisible()) {
        mTestsDock->show();
        mTestsDock->raise();
    } else {
        mTestsDock->raise();
    }
}

void Terminal::onToolButtonIndexClick()
{
    int index = sender()->property("index").toInt();
    if (index < 0 || index >= bufferPackets.size()) return;

    Packet packet = bufferPackets.at(index);

    ui->scrollAreaWidgetContentsCommandUART->layout()->removeWidget(packet.frame);
    packet.frame->deleteLater();
    bufferPackets.removeAt(index);
    if (currentIndexPacket > 0) currentIndexPacket--;

    for (int i = index; i < bufferPackets.size(); i++) {
        Packet& p = bufferPackets[i];
        p.index = i;

        QList<QObject*> children = p.frame->children();
        if (children.size() > 5) {
            QToolButton* toolButtonIndex = qobject_cast<QToolButton*>(children[0]);
            if (toolButtonIndex) {
                toolButtonIndex->setText(QString::number(i + 1));
                toolButtonIndex->setProperty("index", i);
            }

            QPushButton* button = qobject_cast<QPushButton*>(children[3]);
            if (button) button->setProperty("index", i);

            QSpinBox* spinBox = qobject_cast<QSpinBox*>(children[4]);
            if (spinBox) spinBox->setProperty("index", i);

            QCheckBox* checkBox = qobject_cast<QCheckBox*>(children[5]);
            if (checkBox) checkBox->setProperty("index", i);
        }

        if (p.timer) {
            p.timer->setProperty("index", i);
        }
    }
}

void Terminal::onPushButtonExecuteClicked()
{
    int index = sender()->property("index").toInt();
    if (index < 0 || index >= bufferPackets.size()) return;

    Packet& packet = bufferPackets[index];

    if (packet.state) {
        packet.timer->start();
    }
    else {
        QString packetString = packet.command.split(" ").join("");
        QByteArray command = QByteArray::fromHex(packetString.toLatin1());
        mUART->writeData(mUART->buildPacket(command));
    }
}

void Terminal::onSpinBoxRepeatValueChanged(int value)
{
    int index = sender()->property("index").toInt();
    if (index >= 0 && index < bufferPackets.size()) {
        bufferPackets[index].repeat = value;
        if (bufferPackets[index].timer) {
            bufferPackets[index].timer->setInterval(value);
        }
    }
}

void Terminal::onCheckBoxStateToggled(bool state)
{
    int index = sender()->property("index").toInt();
    if (index >= 0 && index < bufferPackets.size()) {
        bufferPackets[index].state = state;
        if (!state && bufferPackets[index].timer && bufferPackets[index].timer->isActive()) {
            bufferPackets[index].timer->stop();
        }
    }
}

void Terminal::onTimerExecuteTimeout()
{
    int index = sender()->property("index").toInt();
    if (index < 0 || index >= bufferPackets.size()) return;

    Packet& packet = bufferPackets[index];

    if (!packet.state) {
        if (packet.timer) packet.timer->stop();
        return;
    }

    QString packetString = packet.command.split(" ").join("");
    QByteArray command = QByteArray::fromHex(packetString.toLatin1());
    if (!mUART->writeData(mUART->buildPacket(command))) {
        packet.timer->stop();
    }
}

void Terminal::onCommandsDockVisibilityChanged(bool visible)
{
    if (visible) {
        ui->pushButtonOpenMacros->setEnabled(false);
    } else {
        ui->pushButtonOpenMacros->setEnabled(true);
    }
}

void Terminal::onTestsDockVisibilityChanged(bool visible)
{
    if (visible) {
        ui->pushButtonOpenTests->setEnabled(false);
    } else {
        ui->pushButtonOpenTests->setEnabled(true);
    }
}

void Terminal::closeEvent(QCloseEvent *event)
{
    saveSettings(filenameSettings);
    event->accept();
}

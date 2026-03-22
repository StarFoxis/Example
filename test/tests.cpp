#include "tests.h"
#include "ui_tests.h"

Tests::Tests(QWidget *parent)
    : QWidget{parent}
    , ui{new Ui::Tests}
{
    ui->setupUi(this);

    initWidgets();
    initConnects();
}

Tests::~Tests()
{
    delete ui;
}

void Tests::initWidgets()
{
    ui->progressBarTests->setValue(0);
    ui->progressBarTests->setAlignment(Qt::AlignCenter);
}

void Tests::initConnects()
{
    connect(ui->pushButtonStartTests, &QPushButton::clicked,
            this, &Tests::onPushButtonStartTests);

    if (ui->tableWidgetModule1) {
        connect(ui->tableWidgetModule1, &TableWidget::signalNewMessage,
                this, [this](const QString& message, MessageType messageType){
                    emit signalNewMessage(message, messageType);
                });
    }

    if (ui->tableWidgetModule2) {
        connect(ui->tableWidgetModule2, &TableWidget::signalNewMessage,
                this, [this](const QString& message, MessageType messageType){
                    emit signalNewMessage(message, messageType);
                });
    }
}

void Tests::setUART(UART *uart)
{
    mUART = uart;

    if (ui->tableWidgetModule1) {
        ui->tableWidgetModule1->setUART(mUART);
    }
    if (ui->tableWidgetModule2) {
        ui->tableWidgetModule2->setUART(mUART);
    }
}

TableWidget* Tests::getCurrentTableWidget()
{
    if (!ui->tabWidgetModules) {
        return nullptr;
    }

    QWidget* currentTab = ui->tabWidgetModules->currentWidget();
    if (!currentTab) {
        return nullptr;
    }

    TableWidget* tableWidget = currentTab->findChild<TableWidget*>();
    return tableWidget;
}

void Tests::saveSettings(QSettings* settings)
{
    if (!settings) return;

    settings->beginGroup("tests");
    // // Сохраняем состояние виджетов тестов
    // if (ui->tableWidgetModule1) {
    //     ui->tableWidgetModule1->saveSettings(settings, "module1");
    // }
    // if (ui->tableWidgetModule2) {
    //     ui->tableWidgetModule2->saveSettings(settings, "module2");
    // }
    settings->endGroup();
}

void Tests::loadSettings(QSettings* settings)
{
    if (!settings) return;

    settings->beginGroup("tests");
    // // Загружаем состояние виджетов тестов
    // if (ui->tableWidgetModule1) {
    //     ui->tableWidgetModule1->loadSettings(settings, "module1");
    // }
    // if (ui->tableWidgetModule2) {
    //     ui->tableWidgetModule2->loadSettings(settings, "module2");
    // }
    settings->endGroup();
}

void Tests::startingTests()
{
    TableWidget* tableWidget = getCurrentTableWidget();

    if (!tableWidget) {
        emit signalNewMessage("Не удалось получить таблицу тестов", Error);
        finishTests();
        return;
    }

    mActiveTests.clear();
    for (int i = 0; i < tableWidget->rowCount(); ++i) {
        if (tableWidget->isTestEnabled(i)) {
            QString name = tableWidget->getName(i);
            if (name.isEmpty()) {
                name = QString("Тест %1").arg(i + 1);
            }
            mActiveTests.append({i, name});
        }
    }

    if (mActiveTests.isEmpty()) {
        emit signalNewMessage("Нет активных тестов для выполнения", Warning);
        finishTests();
        return;
    }

    mCurrentTestIndex = 0;
    mTestIsRunning = true;

    ui->progressBarTests->setValue(0);
    ui->progressBarTests->setMaximum(mActiveTests.size());
    ui->pushButtonStartTests->setText("Остановить");
    if (ui->pushButtonCheckConnect) {
        ui->pushButtonCheckConnect->setEnabled(false);
    }
    ui->tabWidgetModules->setEnabled(false);

    emit signalNewMessage(QString("Начинаем выполнение тестов. Всего тестов: %1")
                              .arg(mActiveTests.size()));

    executeTests();
}

void Tests::stopTests()
{
    if (!mTestIsRunning) {
        return;
    }

    mTestIsRunning = false;
    mActiveTests.clear();
    mCurrentTestIndex = -1;

    ui->pushButtonStartTests->setText("Начать тестирование");
    if (ui->pushButtonCheckConnect) {
        ui->pushButtonCheckConnect->setEnabled(true);
    }
    ui->tabWidgetModules->setEnabled(true);
    ui->progressBarTests->setValue(0);

    emit signalNewMessage("Тестирование остановлено пользователем", Warning);
}

void Tests::finishTests()
{
    if (!mTestIsRunning) {
        return;
    }

    mTestIsRunning = false;
    mActiveTests.clear();
    mCurrentTestIndex = -1;

    ui->pushButtonStartTests->setText("Начать тестирование");
    if (ui->pushButtonCheckConnect) {
        ui->pushButtonCheckConnect->setEnabled(true);
    }
    ui->tabWidgetModules->setEnabled(true);
    ui->progressBarTests->setValue(0);

    emit signalNewMessage("Тестирование завершено", Success);
}

void Tests::executeTests()
{
    if (!mTestIsRunning) {
        return;
    }

    if (mCurrentTestIndex >= mActiveTests.size()) {
        finishTests();
        return;
    }

    TableWidget* tableWidget = getCurrentTableWidget();

    if (!tableWidget) {
        emit signalNewMessage("Не удалось получить таблицу тестов", Error);
        finishTests();
        return;
    }

    int testIndex = mActiveTests[mCurrentTestIndex].first;
    QString testName = mActiveTests[mCurrentTestIndex].second;

    emit signalNewMessage(QString("Выполняется тест %1/%2: %3")
                              .arg(mCurrentTestIndex + 1)
                              .arg(mActiveTests.size())
                              .arg(testName));

    bool result = tableWidget->runTest(testIndex);

    if (result) {
        emit signalNewMessage(QString("Тест \"%1\" выполнен успешно").arg(testName), Success);
    } else {
        emit signalNewMessage(QString("Тест \"%1\" не выполнен").arg(testName), Error);
    }

    ui->progressBarTests->setValue(mCurrentTestIndex + 1);

    mCurrentTestIndex++;

    // Запускаем следующий тест с задержкой
    QTimer::singleShot(500, this, &Tests::executeTests);
}

void Tests::onPushButtonStartTests()
{
    if (!mUART) {
        emit signalNewMessage("UART не инициализирован", Error);
        return;
    }

    if (!mUART->isOpen()) {
        emit signalNewMessage("Проверьте подключение к последовательному порту.", Error);
        return;
    }

    if (mTestIsRunning) {
        stopTests();
        return;
    }

    TableWidget* tableWidget = getCurrentTableWidget();

    if (!tableWidget) {
        emit signalNewMessage("Не удалось получить таблицу тестов", Error);
        return;
    }

    // Проверяем, есть ли активные тесты
    bool hasActiveTests = false;
    for (int i = 0; i < tableWidget->rowCount(); ++i) {
        if (tableWidget->isTestEnabled(i)) {
            hasActiveTests = true;
            break;
        }
    }

    if (!hasActiveTests) {
        emit signalNewMessage("Нет активных тестов. Отметьте тесты для выполнения.", Warning);
        return;
    }

    startingTests();
}

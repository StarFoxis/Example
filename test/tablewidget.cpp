#include "tablewidget.h"
#include <QHeaderView>
#include <QHBoxLayout>

TableWidget::TableWidget(QWidget *parent)
    : QTableWidget{parent}
{
    // Устанавливаем количество колонок
    setColumnCount(3);

    // Устанавливаем заголовки
    QStringList headers;
    headers << "№ теста" << "Запуск" << "Название теста";
    setHorizontalHeaderLabels(headers);

    // Настройка внешнего вида
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    verticalHeader()->setVisible(false);

    // Устанавливаем ширину колонок
    setColumnWidth(0, 60);
    setColumnWidth(1, 60);
    horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    // Запрещаем выделение и редактирование
    setSelectionMode(QAbstractItemView::NoSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(this, &QTableWidget::itemClicked, this, &TableWidget::onItemClicked);
}

void TableWidget::setUART(UART *uart)
{
    mUART = uart;
}

void TableWidget::insertTest(int row, const QString &testName, std::function<bool()> testFunction)
{
    // Вставляем новую строку
    insertRow(row);

    // Устанавливаем номер теста
    QTableWidgetItem *indexItem = new QTableWidgetItem(QString::number(row + 1));
    indexItem->setTextAlignment(Qt::AlignCenter);
    indexItem->setFlags(indexItem->flags() & ~Qt::ItemIsEditable);
    setItem(row, 0, indexItem);

    // Создаём и добавляем CheckBox
    createCheckBox(row);

    // Устанавливаем название теста
    QTableWidgetItem *nameItem = new QTableWidgetItem(testName);
    nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
    setItem(row, 2, nameItem);

    mTestNames.insert(row, testName);

    // Сохраняем функцию теста
    if (mTestFunctions.size() <= row) {
        mTestFunctions.resize(row + 1);
    }
    mTestFunctions[row] = testFunction;
}

void TableWidget::createCheckBox(int row)
{
    // Создаём виджет-контейнер для центрирования CheckBox
    QWidget *widget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);

    // Создаём CheckBox
    QCheckBox *checkBox = new QCheckBox();
    checkBox->setProperty("row", row);
    checkBox->setChecked(false);  // По умолчанию тест выключен

    // Добавляем CheckBox в layout
    layout->addWidget(checkBox);
    widget->setLayout(layout);

    // Устанавливаем виджет в таблицу
    setCellWidget(row, 1, widget);

    // Сохраняем указатель на CheckBox
    if (mCheckBoxes.size() <= row) {
        mCheckBoxes.resize(row + 1);
    }
    mCheckBoxes[row] = checkBox;
}

void TableWidget::onItemClicked(QTableWidgetItem *item)
{
    if (!item) return;

    int row = item->row();

    // Проверяем, что строка существует
    if (row < 0 || row >= mCheckBoxes.size()) return;

    // Получаем checkbox для этой строки
    QCheckBox *checkBox = mCheckBoxes[row];
    if (checkBox) {
        // Переключаем состояние
        checkBox->setChecked(!checkBox->isChecked());
    }
}

bool TableWidget::runTest(int index)
{
    if (mUART == nullptr) {
        emit signalNewMessage("Нет связи с последовательным портом, перезапустите программу.", Error);
        return false;
    }

    if (index < 0 || index >= mTestFunctions.size()) {
        emit signalNewMessage("Выход за пределы массива.", Error);
        return false;
    }

    if (!mTestFunctions[index]) {
        emit signalNewMessage(QString("Теста с номером: %1 не существует.").arg(index), Error);
        return false;
    }

    return mTestFunctions[index]();
}

bool TableWidget::isTestEnabled(int row) const
{
    if (row < 0 || row >= mCheckBoxes.size()) {
        return false;
    }

    if (!mCheckBoxes[row]) {
        return false;
    }

    return mCheckBoxes[row]->isChecked();
}

QString TableWidget::getName(int row)
{
    if (row < 0 || row >= mTestNames.size()) {
        return "";
    }
    return mTestNames.at(row);
}

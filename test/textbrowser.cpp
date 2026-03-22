// TextBrowser.cpp
#include "TextBrowser.h"
#include <QDebug>

TextBrowser::TextBrowser(QWidget *parent)
    : QTextBrowser(parent)
    , m_buttonMargin(10)
    , m_opacityHover(1.0f)
    , m_opacityNormal(0.5f)
    , m_isHovering(false)
    , m_zoomStep(0)
    , m_baseFontSize(10)  // Базовый размер шрифта
    , m_minFontSize(8)    // Минимальный размер
    , m_maxFontSize(24)   // Максимальный размер
    , m_zoomIncrement(1)  // Увеличивать/уменьшать на 1 пункт
{
    setupButton();
    setupOpacityEffect();

    // Устанавливаем tracking для отслеживания движения мыши
    setMouseTracking(true);

    // Сохраняем базовый размер шрифта
    m_baseFontSize = font().pointSize();
    if (m_baseFontSize <= 0) {
        // Если pointSize не установлен (используется pixelSize), конвертируем
        m_baseFontSize = 10;
        QFont defaultFont = font();
        defaultFont.setPointSize(m_baseFontSize);
        setFont(defaultFont);
    }

    // Устанавливаем политику фокуса для получения событий клавиатуры
    setFocusPolicy(Qt::StrongFocus);
}

void TextBrowser::setupButton()
{
    m_clearButton = new QPushButton("Очистить", this);
    m_clearButton->setObjectName("clearButton");
    // m_clearButton->setCursor(Qt::PointingHandCursor);

    m_clearButton->setStyleSheet(
        "QPushButton#clearButton {"
        "   background-color: #f0f0f0;"
        "   border: 1px solid #c0c0c0;"
        "   border-radius: 4px;"
        "   padding: 4px 12px;"
        "   font-size: 12px;"
        "   color: #333;"
        "   min-width: 60px;"
        "}"
        "QPushButton#clearButton:hover {"
        "   background-color: #e0e0e0;"
        "   border-color: #0078d7;"
        "   color: #0078d7;"
        "}"
        "QPushButton#clearButton:pressed {"
        "   background-color: #d0d0d0;"
        "}"
        );

    connect(m_clearButton, &QPushButton::clicked,
            this, &TextBrowser::onClearButtonClicked);

    connect(verticalScrollBar(), &QScrollBar::rangeChanged,
            this, &TextBrowser::onScrollBarRangeChanged);

    m_clearButton->show();
    updateButtonPosition();
}

void TextBrowser::setupOpacityEffect()
{
    m_opacityEffect = new QGraphicsOpacityEffect(m_clearButton);
    m_opacityEffect->setOpacity(m_opacityNormal);
    m_clearButton->setGraphicsEffect(m_opacityEffect);

    m_opacityAnimation = new QPropertyAnimation(this, "buttonOpacity");
    m_opacityAnimation->setDuration(200);
    m_opacityAnimation->setEasingCurve(QEasingCurve::InOutQuad);
}

float TextBrowser::buttonOpacity() const
{
    return m_opacityEffect ? m_opacityEffect->opacity() : 1.0f;
}

void TextBrowser::setButtonOpacity(float opacity)
{
    if (m_opacityEffect) {
        m_opacityEffect->setOpacity(opacity);
    }
}

void TextBrowser::animateOpacity(float targetOpacity)
{
    if (m_opacityAnimation) {
        m_opacityAnimation->stop();
        m_opacityAnimation->setEndValue(targetOpacity);
        m_opacityAnimation->start();
    }
}

void TextBrowser::enterEvent(QEnterEvent *event)
{
    QTextBrowser::enterEvent(event);
    m_isHovering = true;
    animateOpacity(m_opacityHover);
}

void TextBrowser::leaveEvent(QEvent *event)
{
    QTextBrowser::leaveEvent(event);
    m_isHovering = false;
    animateOpacity(m_opacityNormal);
}

void TextBrowser::resizeEvent(QResizeEvent *event)
{
    QTextBrowser::resizeEvent(event);
    updateButtonPosition();
}

void TextBrowser::showEvent(QShowEvent *event)
{
    QTextBrowser::showEvent(event);
    updateButtonPosition();
}

void TextBrowser::updateButtonPosition()
{
    if (!m_clearButton)
        return;

    int buttonWidth = m_clearButton->sizeHint().width();
    int buttonHeight = m_clearButton->sizeHint().height();

    int x = width() - buttonWidth - m_buttonMargin;
    int y = height() - buttonHeight - m_buttonMargin;

    if (verticalScrollBar()->isVisible()) {
        x -= verticalScrollBar()->width();
    }

    if (horizontalScrollBar()->isVisible()) {
        y -= horizontalScrollBar()->height();
    }

    m_clearButton->move(x, y);
}

void TextBrowser::onClearButtonClicked()
{
    clear();
    emit signalClearHistory();
}

void TextBrowser::onScrollBarRangeChanged(int min, int max)
{
    Q_UNUSED(min);
    Q_UNUSED(max);
    updateButtonPosition();
}

void TextBrowser::setClearButtonVisible(bool visible)
{
    m_clearButton->setVisible(visible);
    if (visible) {
        updateButtonPosition();
        if (m_isHovering) {
            animateOpacity(m_opacityHover);
        } else {
            animateOpacity(m_opacityNormal);
        }
    }
}

bool TextBrowser::isClearButtonVisible() const
{
    return m_clearButton->isVisible();
}

void TextBrowser::setOpacityAnimationDuration(int ms)
{
    if (m_opacityAnimation) {
        m_opacityAnimation->setDuration(ms);
    }
}

void TextBrowser::setButtonOpacityHover(float opacity)
{
    m_opacityHover = qBound(0.0f, opacity, 1.0f);
    if (m_isHovering && m_clearButton->isVisible()) {
        animateOpacity(m_opacityHover);
    }
}

void TextBrowser::setButtonOpacityNormal(float opacity)
{
    m_opacityNormal = qBound(0.0f, opacity, 1.0f);
    if (!m_isHovering && m_clearButton->isVisible()) {
        animateOpacity(m_opacityNormal);
    }
}

// Реализация методов для управления размером шрифта
void TextBrowser::zoomIn(int step)
{
    int newStep = m_zoomStep + step;
    if (newStep > (m_maxFontSize - m_baseFontSize) / m_zoomIncrement) {
        newStep = (m_maxFontSize - m_baseFontSize) / m_zoomIncrement;
    }

    if (newStep != m_zoomStep) {
        m_zoomStep = newStep;
        updateFontSize();
    }
}

void TextBrowser::zoomOut(int step)
{
    int newStep = m_zoomStep - step;
    if (newStep < (m_minFontSize - m_baseFontSize) / m_zoomIncrement) {
        newStep = (m_minFontSize - m_baseFontSize) / m_zoomIncrement;
    }

    if (newStep != m_zoomStep) {
        m_zoomStep = newStep;
        updateFontSize();
    }
}

void TextBrowser::zoomReset()
{
    if (m_zoomStep != 0) {
        m_zoomStep = 0;
        updateFontSize();
    }
}

void TextBrowser::setZoomStep(int step)
{
    int maxStep = (m_maxFontSize - m_baseFontSize) / m_zoomIncrement;
    int minStep = (m_minFontSize - m_baseFontSize) / m_zoomIncrement;

    step = qBound(minStep, step, maxStep);

    if (step != m_zoomStep) {
        m_zoomStep = step;
        updateFontSize();
    }
}

int TextBrowser::getZoomStep() const
{
    return m_zoomStep;
}

void TextBrowser::setMinFontSize(int minSize)
{
    m_minFontSize = qMax(4, minSize);
    // Проверяем, не выходит ли текущий размер за новые границы
    int currentSize = m_baseFontSize + m_zoomStep * m_zoomIncrement;
    if (currentSize < m_minFontSize) {
        m_zoomStep = (m_minFontSize - m_baseFontSize) / m_zoomIncrement;
        updateFontSize();
    }
}

void TextBrowser::setMaxFontSize(int maxSize)
{
    m_maxFontSize = qMin(72, maxSize);
    // Проверяем, не выходит ли текущий размер за новые границы
    int currentSize = m_baseFontSize + m_zoomStep * m_zoomIncrement;
    if (currentSize > m_maxFontSize) {
        m_zoomStep = (m_maxFontSize - m_baseFontSize) / m_zoomIncrement;
        updateFontSize();
    }
}

void TextBrowser::updateFontSize()
{
    int newSize = m_baseFontSize + m_zoomStep * m_zoomIncrement;
    newSize = qBound(m_minFontSize, newSize, m_maxFontSize);

    QFont newFont = font();
    newFont.setPointSize(newSize);
    setFont(newFont);
    setStyleSheet(QString("font-size: %1pt").arg(newSize));

    emit signalFontSizeChanged(newSize);

    // Обновляем позицию кнопки после изменения размера шрифта
    updateButtonPosition();
}

void TextBrowser::keyPressEvent(QKeyEvent *event)
{
    // Обработка Ctrl + "+" или Ctrl + "=" (увеличение)
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) {
            zoomIn();
            event->accept();
            return;
        }
        // Обработка Ctrl + "-" (уменьшение)
        else if (event->key() == Qt::Key_Minus) {
            zoomOut();
            event->accept();
            return;
        }
        // Обработка Ctrl + "0" (сброс)
        else if (event->key() == Qt::Key_0) {
            zoomReset();
            event->accept();
            return;
        }
    }

    // Передаем событие дальше, если не обработали
    QTextBrowser::keyPressEvent(event);
}

void TextBrowser::wheelEvent(QWheelEvent *event)
{
    // Опционально: поддержка Ctrl + колесико мыши для изменения размера
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->angleDelta().y() > 0) {
            zoomIn();
        } else if (event->angleDelta().y() < 0) {
            zoomOut();
        }
        event->accept();
        return;
    }

    // Если Ctrl не нажат, передаем событие дальше (обычная прокрутка)
    QTextBrowser::wheelEvent(event);
}

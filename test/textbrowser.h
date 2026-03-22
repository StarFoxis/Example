// TextBrowser.h
#ifndef TEXTBROWSER_H
#define TEXTBROWSER_H

#include <QTextBrowser>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollBar>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEvent>
#include <QEnterEvent>
#include <QWheelEvent>
#include <QKeyEvent>

class TextBrowser : public QTextBrowser
{
    Q_OBJECT
    Q_PROPERTY(float buttonOpacity READ buttonOpacity WRITE setButtonOpacity)

public:
    explicit TextBrowser(QWidget *parent = nullptr);

    void setClearButtonVisible(bool visible);
    bool isClearButtonVisible() const;

    void setOpacityAnimationDuration(int ms);
    void setButtonOpacityHover(float opacity);
    void setButtonOpacityNormal(float opacity);

    // Методы для управления размером шрифта
    void zoomIn(int step = 1);
    void zoomOut(int step = 1);
    void zoomReset();
    void setZoomStep(int step);
    int getZoomStep() const;
    void setMinFontSize(int minSize);
    void setMaxFontSize(int maxSize);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;  // Опционально: поддержка Ctrl+колесико

private:
    void setupButton();
    void setupOpacityEffect();
    void updateButtonPosition();
    void animateOpacity(float targetOpacity);
    void updateFontSize();  // Обновляет размер шрифта на основе текущего шага

    float buttonOpacity() const;
    void setButtonOpacity(float opacity);

private slots:
    void onClearButtonClicked();
    void onScrollBarRangeChanged(int min, int max);

private:
    QPushButton *m_clearButton;
    QGraphicsOpacityEffect *m_opacityEffect;
    QPropertyAnimation *m_opacityAnimation;

    int m_buttonMargin;
    float m_opacityHover;      // Прозрачность при наведении (1.0 = полностью видима)
    float m_opacityNormal;     // Прозрачность когда курсор не на виджете (0.5 = полупрозрачна)
    bool m_isHovering;

    // Параметры для масштабирования шрифта
    int m_zoomStep;            // Текущий шаг масштабирования (0 = базовый)
    int m_baseFontSize;        // Базовый размер шрифта
    int m_minFontSize;         // Минимальный размер шрифта
    int m_maxFontSize;         // Максимальный размер шрифта
    int m_zoomIncrement;       // Шаг изменения размера шрифта

signals:
    void signalClearHistory();
    void signalFontSizeChanged(int fontSize);  // Сигнал об изменении размера шрифта
};

#endif // TEXTBROWSER_H

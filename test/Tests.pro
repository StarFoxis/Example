QT       += core gui serialport svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    global.cpp \
    main.cpp \
    modules/module1.cpp \
    modules/module2.cpp \
    terminal.cpp \
    tests.cpp \
    uart.cpp \
    widgets/tablewidget.cpp \
    widgets/textbrowser.cpp

HEADERS += \
    global.h \
    modules/module1.h \
    modules/module2.h \
    terminal.h \
    tests.h \
    uart.h \
    widgets/tablewidget.h \
    widgets/textbrowser.h

FORMS += \
    terminal.ui \
    tests.ui

RESOURCES += \
    tests.qrc

DISTFILES += \
    dark.qss \
    light.qss

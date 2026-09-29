QT -= gui
QT += core

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = AsyncCalculator
TEMPLATE = app

SOURCES += \
    main.cpp \
    calculator.cpp

HEADERS += \
    calculator.h
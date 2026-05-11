TEMPLATE = app
TARGET = HomeSmartHomeQT

QT += core gui qml quick quickcontrols2 network
CONFIG += c++17

SOURCES += \
    src/main.cpp \
    src/TcpClient.cpp \
    src/HomeState.cpp

HEADERS += \
    src/TcpClient.h \
    src/HomeState.h

RESOURCES += \
    qml.qrc

QMAKE_CXXFLAGS += -Wall -Wextra


#-------------------------------------------------
#
# Project created by QtCreator 2023-08-22T20:06:19
#
#-------------------------------------------------
# 添加Qt核心、GUI、窗口部件和打印支持模块
QT       += core gui
QT       += widgets printsupport
QT       += network

# 如果Qt主要版本号大于4，则添加窗口部件模块
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# 指定项目名称和模板为应用程序
TARGET = TablePilot
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
# 定义编译器警告行为为发出已弃用特性的警告
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
# 配置C++标准为C++11
CONFIG += c++11
win32-g++:QMAKE_CXXFLAGS += -Wa,-mbig-obj

# 指定源文件、头文件和界面文件
SOURCES += \
        main.cpp \
        mainwindow.cpp \
        textformat.cpp \
        webresearchdock.cpp \
        qcustomplot.cpp \

HEADERS += \
        mainwindow.h \
        textformat.h \
        webresearchdock.h \
        qcustomplot.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
# 在不同平台上指定部署目标路径
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
